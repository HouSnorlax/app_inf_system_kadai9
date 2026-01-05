#include <M5StickCPlus.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t ir_recv_pin = 33;
const uint16_t CaptureBufferSize = 1024;
const uint8_t Timeout = 50;
const uint16_t MinUnknownSize = 12;

IRrecv irrecv(ir_recv_pin, CaptureBufferSize, Timeout, true); // 受信オブジェクトの作成
decode_results results; // 受信した情報

void setup()
{
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.print("IR Recv Mode");
    Serial.begin(115200);
    irrecv.enableIRIn(); // 受信開始
}

void loop()
{
    if (irrecv.decode(&results)) {
        Serial.println("--- Signal Received ---");
        String code = resultToSourceCode(&results);
        Serial.println(code);
        Serial.println("");
        irrecv.resume(); // 次の受信へ
    }
}