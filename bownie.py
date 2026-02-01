import gc
import image
import lcd
import sensor
import sys
import time
import uos
import KPU as kpu
from fpioa_manager import *
from machine import I2C
from Maix import I2S, GPIO
from machine import UART
from board import board_info
#
# initialize
#
lcd.init()
lcd.rotation(2)
#M5StickV GPIO_UART
fm.register(35, fm.fpioa.UART2_TX, force=True)
fm.register(34, fm.fpioa.UART2_RX, force=True)
uart_Port = UART(UART.UART2, 115200,8,0,0, timeout=1000, read_buf_len=4096)
#M5StickV Button
fm.register(board_info.BUTTON_A, fm.fpioa.GPIO1)
but_a=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!
fm.register(board_info.BUTTON_B, fm.fpioa.GPIO2)
but_b = GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP) #PULL_UP is required here!

def send(img_buf, color):
    lcd.draw_string(40, 40, "Sending...", lcd.WHITE, color)
    print('[Info]: Sending...')
    img_buf = img_buf.resize(128,128)
    img_size1 = (img_buf.size()& 0xFF0000)>>16
    img_size2 = (img_buf.size()& 0x00FF00)>>8
    img_size3 = (img_buf.size()& 0x0000FF)>>0
    data_packet = bytearray([0xFF,0xD8,0xEA,0x01,img_size1,img_size2,img_size3,0x00,0x00,0x00])
    uart_Port.write(data_packet)
    uart_Port.write(img_buf)
    del img_buf
    print('[Info]: Done.')

def initialize_camera():
    err_counter = 0
    while 1:
        try:
            sensor.reset() #Reset sensor may failed, let's try some times
            break
        except:
            err_counter = err_counter + 1
            if err_counter == 20:
                lcd.draw_string(lcd.width()//2-100,lcd.height()//2-4, "Error: Sensor Init Failed", lcd.WHITE, lcd.RED)
            time.sleep(0.1)
            continue

    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA) #QVGA=320x240
    sensor.skip_frames(10)
    sensor.run(1)

#
# main
#
initialize_camera()
task = kpu.load(0x600000)
classes = ['hand','background']
anchor = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437, 6.92275, 6.718375, 9.01025)
# Anchor data is for bbox, extracted from the training sets.
kpu.init_yolo2(task, 0.35, 0.3, 5, anchor)

try:
    while(True):
        gc.collect()
        time.sleep(0.01)
        img = sensor.snapshot()
        code_obj = kpu.run_yolo2(task, img)
        if code_obj: # object detected
            max_id = 0
            max_rect = 0
            max_i = code_obj[0]
            for i in code_obj:
                id = i.classid()
                rect_size = i.w() * i.h()
                if rect_size > max_rect:
                    max_rect = rect_size
                    max_id = id
                    max_i = i
            if max_i.w() > max_i.h():
                max_x = max_i.x()
                max_y = max_i.y() - (max_i.w()-max_i.h())//2
                max_w = max_i.w()
                max_h = max_i.w()
            else:
                max_w = max_i.h()
                max_h = max_i.h()
                max_x = max_i.x() - (max_i.h()-max_i.w())//2
                max_y = max_i.y()

            img.draw_rectangle(max_x,max_y,max_w,max_h,color=(255,255,0))
            text = ' ' + classes[max_id] + ' (' + str(int(max_i.value()*100)) + '%) '
            lcd.display(img)
            lcd.draw_string(40, 40, text, lcd.WHITE, lcd.BLACK)
            print('[Result]: ' + text)
            #IF Button A Push Then Image Send UART
            if (classes[max_id] == "hand" and max_i.value() >= 0.5):
                lcd.display(img)
                send(img.copy((max_x,max_y,max_w,max_h)), lcd.RED)
        else:
            lcd.display(img)

        if but_b.value() == 0:
            img.draw_rectangle(96,56,128,128,color=(255,255,255))
            lcd.display(img)
            send(img.copy((96,56,128,128)), lcd.BLUE)

except KeyboardInterrupt:
    kpu.deinit(task)
    sys.exit()
