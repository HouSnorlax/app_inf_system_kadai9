#include <M5StickCPlus.h>
#include <IRremoteESP8266.h>
#include <ir_Panasonic.h>

int ir_send_pin = 9;  // IR送信ピン

IRPanasonicAc ac(ir_send_pin);

uint8_t state[27] = {
    0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06, 0x02, 0x20, 
    0xE0, 0x04, 0x00, 0x40, 0x2E, 0x80, 0xAF, 0x00, 0x00, 0x06, 
    0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x8F
};
bool power = false;
uint8_t degrees = 23;

void setup() {
    M5.begin();
    ac.begin();
    
    ac.setRaw(state);

    M5.Lcd.setRotation(3);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("IR Ready");
}

void loop() {
    M5.update();
    
    if (M5.BtnA.wasPressed()) {
        if (power){
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Sending...");
            
            ac.off();
            ac.send();
            
            delay(500);
            M5.Lcd.println("Power OFF");
            power = false;
        }
        else {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Sending...");
            
            ac.on();
            ac.send();

            delay(500);
            M5.Lcd.println("Power ON");
            power = true;
        }
    }

    if (M5.BtnB.wasPressed()){
        if(power){
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Sending...");
            
            degrees++;
            ac.setTemp(degrees);
            ac.send();
            
            delay(500);
            M5.Lcd.println("Turn up");
        }
    }

        if (M5.Axp.GetBtnPress() == 2){
        if(power){
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Sending...");
            
            degrees--;
            ac.setTemp(degrees);
            ac.send();
            
            delay(500);
            M5.Lcd.println("Turn down");
        }
    }
}