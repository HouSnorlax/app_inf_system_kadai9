#include <M5StickCPlus.h>
#include <IRremoteESP8266.h>
#include <ir_Panasonic.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

int ir_send_pin = 9;  // IR送信ピン
int led_pin = 10; //G10

/* エアコン操作用初期設定 */
IRPanasonicAc ac(ir_send_pin);

uint8_t state[27] = {
    0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06, 0x02, 0x20, 
    0xE0, 0x04, 0x00, 0x40, 0x2E, 0x80, 0xAF, 0x00, 0x00, 0x06, 
    0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x8F
}; // 赤外線データ
bool power = false;
uint8_t degrees = 23;

/* 画像データ送信初期設定*/
WiFiClient espClient;
HTTPClient myHttp;

const char ssid[] = "";
const char* password = "";
const String url = "http://000.000.000.000:5000";
const String act = "/recog";


static const int x = 128, y = 128;
int xy = x * y;
static const byte packet_begin[3] = { 0xFF, 0xD8, 0xEA };
static const byte head[66] = {0x42, 0x4d,
                              0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00,
                              0x00, 0x00,
                              0x42, 0x00, 0x00, 0x00,
                              0x28, 0x00, 0x00, 0x00,
                              0x80, 0x00, 0x00, 0x00, //width 128 = 0x80
                              0x80, 0x00, 0x00, 0x00, //height 128 = 0x80
                              0x01, 0x00,
                              0x10, 0x00,
                              0x03, 0x00, 0x00, 0x00,
                              0x00, 0x80, 0x00, 0x00, //size 128*128*2 = 0x8000
                              0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x00, 0xf8, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00
                             };
byte* rx_buffer;
byte rx_buffer1[10];
uint16_t* rx_buffer2;
byte* uiB;

void setupWifi();

void setup() {
    M5.begin();
    ac.begin();
    
    ac.setRaw(state);

    M5.Axp.ScreenBreath(100); //brightness = MAX
    pinMode(LED_PIN, OUTPUT);

    setupWifi();
    Serial2.begin(115200, SERIAL_8N1, 32, 33);

    rx_buffer  = (byte *)malloc(sizeof(byte) * 2 * xy);
    uiB = (byte *)malloc(sizeof(byte) * (2 * xy + 66));
    rx_buffer2 = (uint16_t *)malloc(sizeof(uint16_t) * xy);
    for (int i = 0; i < 66; i++) {
        uiB[i] = head[i];
    }
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


void setupWifi() {
  delay(10);
  digitalWrite(LED_PIN, LOW);
  WiFi.begin(ssid); //Start Wifi connection.

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
  }
  digitalWrite(LED_PIN, HIGH);
}
