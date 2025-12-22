/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
 * @Hardwares: M5Core + Unit IR
 * @Platform Version: Arduino M5Stack Board Manager v2.1.3
 * @Dependent Library:
 * M5Stack@^0.4.6: https://github.com/m5stack/M5Stack
 */

#include <M5StickCPlus.h>

int ir_recv_pin = 36;  // set the input pin. 
int ir_send_pin = 26;

int last_recv_value = 0;
int cur_recv_value  = 0;

void setup()
{
    M5.begin();
    pinMode(ir_recv_pin, INPUT);
    pinMode(ir_send_pin, OUTPUT);
    // send infrared light.
    // now, you can see the infrared light through mobile phone camera.
    digitalWrite(ir_send_pin, 1);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Test for IR receiver: ");
}

void loop()
{
    // now, once you press the button on a remote controller to send infrared
    // light.   the screen will display
    // "detected!" 
    cur_recv_value = digitalRead(ir_recv_pin);
    if (last_recv_value != cur_recv_value) {
        M5.Lcd.setCursor(0, 25);
        M5.Lcd.fillRect(0, 25, 150, 25, BLACK);
        if (cur_recv_value == 0) {  // 0: detected 1: not detected, 
            M5.Lcd.print("detected!");
        }
        last_recv_value = cur_recv_value;
    }
    Serial.println(cur_recv_value);
}