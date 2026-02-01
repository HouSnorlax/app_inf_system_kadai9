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
}; // 赤外線解析によって取得データ
bool power = false;
uint8_t degrees = 23;

/* 画像データ送信初期設定*/
WiFiClient espClient;
HTTPClient myHttp;

const char ssid[] = "";
const char* password = "";
const String url = "http://192.168.10.105:5000";
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
    pinMode(led_pin, OUTPUT);

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
    
    if (Serial2.available()) {
        int rx_size = Serial2.readBytes(rx_buffer1, 10);
        if (rx_size == 10) {   //packet receive of packet_begin
            if ((rx_buffer1[0] == packet_begin[0]) && (rx_buffer1[1] == packet_begin[1]) && (rx_buffer1[2] == packet_begin[2])) {
                rx_size = Serial2.readBytes(rx_buffer, 2 * xy);
                //Serial.println(rx_size);
                for (int i = 0; i < xy; i++) {
                    rx_buffer2[i] = (rx_buffer[2 * i] << 8) + rx_buffer[2 * i + 1];
                    uiB[66 + 2 * i] = rx_buffer[2 * i + 1];
                    uiB[66 + 2 * i + 1] = rx_buffer[2 * i];
                }
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.drawBitmap(108, 4, x, y, rx_buffer2);

                myHttp.begin(url + act);
                myHttp.addHeader("Content-Type", "application/octet-stream");
                int32_t ret = (int32_t)myHttp.POST(uiB, 2 * xy + 66);

                if (ret == HTTP_CODE_OK) {
                    String response = myHttp.getString();
                    //jsonドキュメントの作成 make JSON document
                    const size_t capacity = 768;
                    DynamicJsonDocument doc(capacity);
                    DeserializationError err = deserializeJson(doc, response);
                    
                    int command = doc["command"];

                    
                    M5.Lcd.fillScreen(BLACK);
                    M5.Lcd.setCursor(0, 0);
                    M5.Lcd.setTextSize(3);

                    // Gemini APIからの結果によってエアコンを操作
                    switch (command) {
                    case 1:
                        //電源ON
                        if(!power){
                            M5.Lcd.println("Sending...");
                            
                            ac.on();
                            ac.send();

                            delay(500);
                            M5.Lcd.println("Power ON");
                            power = true;
                        } else {
                            M5.Lcd.println("Now Power ON");
                        }   
                        break;
                    case 2:
                        //電源OFF
                        if(power){
                            M5.Lcd.println("Sending...");
                            
                            ac.off();
                            ac.send();
                            
                            delay(500);
                            M5.Lcd.println("Power OFF");
                            power = false;
                        } else {
                            M5.Lcd.println("Now Power OFF");
                        }
                        break;
                    case 3:
                        //温度上昇
                        if(power){
                            M5.Lcd.println("Sending...");
                            
                            degrees++;
                            ac.setTemp(degrees);
                            ac.send();
                            
                            delay(500);
                            M5.Lcd.printf("Set Temp: %d C\r\n", degrees);
                        } else {
                            M5.Lcd.println("Need Power ON");
                        }
                        break;
                    case 4:
                        //温度低下
                        if(power){
                            M5.Lcd.println("Sending...");
                            
                            degrees--;
                            ac.setTemp(degrees);
                            ac.send();
                            
                            delay(500);
                            M5.Lcd.printf("Set Temp: %d C\r\n", degrees);
                        } else {
                            M5.Lcd.println("Need Power ON");
                        }
                        break;
                    default:
                        M5.Lcd.println("WAIT"); // 何もしない または エラー
                        break;
                    }
                } else {
                    Serial.println(myHttp.errorToString(ret).c_str());
                    M5.Lcd.setCursor(140, 40);
                    M5.Lcd.print("HTTP Error");
                }

                    myHttp.end();
            }
        }
    }
}
void setupWifi() {
    delay(10);
    digitalWrite(led_pin, LOW);
    WiFi.begin(ssid, password); //Start Wifi connection.

    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        digitalWrite(led_pin, HIGH);
        delay(250);
        digitalWrite(led_pin, LOW);
    }
    digitalWrite(led_pin, HIGH);
}
