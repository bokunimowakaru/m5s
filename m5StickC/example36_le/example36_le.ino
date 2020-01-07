/*******************************************************************************
Example 36(=32+4): ESP32 ケチケチ運転術 for M5StickC
内蔵電池で動作するIoTセンサ用の基本形です。
※現時点では待機電流が多い(20mA)、M5StickCライブラリの一部が未実装

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5StickC.h>                           // M5StickC用ライブラリ
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define SLEEP_P 30*1000000ul                    // スリープ時間 30秒(uint32_t)
#define DEVICE "adcnv_1,"                       // デバイス名(5字+"_"+番号+",")

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(M5_LED,OUTPUT);                     // LEDのIOを出力に設定
    M5.begin();                                 // M5StickC用Lcdライブラリの起動
    M5.Axp.ScreenBreath(7+1);                   // LCDの輝度を1に設定
    M5.Lcd.setRotation(1);                      // LCDを横向き表示に設定
    M5.Lcd.println("ESP32 eg.04 LE");           // タイトルと起動理由を表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
}

void loop() {
    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    int batt,battI;                             // 整数型変数battを定義
    IPAddress IP = WiFi.localIP();              // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    
    batt = (int)(M5.Axp.GetVbatData() * 1.1);   // 電池電圧を取得
    battI= M5.Axp.GetIdischargeData();          // 放電電流を取得
    M5.Lcd.printf("%d,%d ",batt,battI);         // 変数battの値を表示
    if(WiFi.status() == WL_CONNECTED){          // 接続に成功
        digitalWrite(M5_LED,HIGH);              // LED OFF
        udp.beginPacket(IP, PORT);              // UDP送信先を設定
        udp.print(DEVICE);                      // デバイス名を送信
        udp.print(batt);                        // 湿度値を送信
        udp.print(", ");                        // カンマを送信
        udp.print(battI);                       // 変数battの値を送信
        udp.print(", ");                        // カンマを送信
        udp.println(millis()/1000 + 0.2,1);     // 起動後の秒数を送信
        udp.endPacket();                        // UDP送信の終了(実際に送信する)
        if(!M5.BtnA.read()) sleep();            // Sleepへ
    }else{
        digitalWrite(M5_LED,!digitalRead(M5_LED));      // LEDの点滅
    }
    delay(100);                                 // 表示の更新間隔 0.1秒
}

void sleep(){
    M5.Axp.ScreenBreath(0);
    M5.Lcd.fillScreen(BLACK);
//  M5.Lcd.sleep();                             // CPP未実装 M5StickC Ver.0.1.1
//  M5.Axp.LightSleep(SLEEP_P);                 // HOMEキーで起動せず Ver.0.1.1
//  M5.Axp.DeepSleep(SLEEP_P);                  // HOMEキーで起動せず Ver.0.1.1

    // RTCクロックを停止 Wire1 0x51 削減効果見えず
    Wire1.beginTransmission(0x51);
    Wire1.write(0x10);                          // PCF8563 RTC clock is stopped
    Wire1.write(0x00);
    Wire1.endTransmission();
    
    // LCDコントローラをスリープ 1.2 mA削減
    bool swap = M5.Lcd.getSwapBytes();
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.writecommand(ST7735_SLPIN);
    M5.Lcd.setSwapBytes(swap);
    
    // LCDバックライト用電源をOFF
    M5.Axp.SetLDO2(false);                      // LCDバックライト用電源
    delay(200);                                 // 送信待ち時間
    
    pinMode(BUTTON_A_PIN,INPUT_PULLUP);
    while(digitalRead(BUTTON_A_PIN) == LOW);
    TimerWakeUp_setExternalInput((gpio_num_t)BUTTON_A_PIN, LOW);
    TimerWakeUp_setSleepTime((int)(SLEEP_P/1000000ul));
    TimerWakeUp_sleep();
}
