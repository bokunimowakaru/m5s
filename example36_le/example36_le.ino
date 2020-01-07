/*******************************************************************************
Example 36(=32+4): ESP32 ケチケチ運転術 for M5Stack
内蔵電池で動作するIoTセンサ用の基本形です。

※現時点では待機電力が大きい43mW（8.5mA×5V）
※保有するM5Stackの電源ICがI2C通信に対応していなかった：
　http://community.m5stack.com/topic/878/solved-can-t-find-ip5306-i2c-address/14

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリ
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define PIN_AIN 36                              // GPIO36をアナログ入力に
#define SLEEP_P 30*1000000ul                    // スリープ時間 30秒(uint32_t)
#define DEVICE "adcnv_3,"                       // デバイス名(5字+"_"+番号+",")

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.Power.begin();                           // M5Stack用Powerライブラリの起動
    pinMode(PIN_AIN,INPUT);                     // アナログ入力の設定
    M5.Lcd.begin();                             // M5Stack用Lcdライブラリの起動
    M5.Lcd.setTextSize(2);                      // 文字表示サイズを2倍に設定
    M5.Lcd.println("ESP32 eg.04 LE");           // タイトルと起動理由を表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
}

void loop() {
    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    int batt;                                   // 整数型変数battを定義
    IPAddress IP = WiFi.localIP();              // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    
    batt = M5.Power.getBatteryLevel();          // 電池電圧を取得
    if(batt < 0){                               // I2C非対応の電源ICだったとき
        batt = (int)mvAnalogIn(PIN_AIN, 0.05);  // AINの電圧を取得
    }
    M5.Lcd.printf("%d ",batt);                  // 変数battの値を表示
    if(WiFi.status() == WL_CONNECTED){          // 接続に成功
        udp.beginPacket(IP, PORT);              // UDP送信先を設定
        udp.print(DEVICE);                      // デバイス名を送信
        udp.print(batt);                        // 変数battの値を送信
        udp.print(", ");                        // カンマを送信
        udp.println(millis()/1000 + 0.2,1);     // 起動後の秒数を送信
        udp.endPacket();                        // UDP送信の終了(実際に送信する)
        if(!M5.BtnA.read()) sleep();            // Sleepへ
    }
    delay(100);                                 // 表示の更新間隔 0.1秒
}

void sleep(){
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.sleep();
    delay(200);                                 // 送信待ち時間
    
    M5.Power.setWakeupButton(BUTTON_A_PIN);
    M5.Power.deepSleep(SLEEP_P);
}

float mvAnalogIn(uint8_t PIN){
    return mvAnalogIn(PIN, 0.0);            // 動作最小電圧 0.0 ～ 0.1(V)程度
//  return mvAnalogIn(PIN, 1.075584e-1);
}

float mvAnalogIn(uint8_t PIN, float offset){
    int in0,in3;
    float ad0,ad3;
    
    analogSetPinAttenuation(PIN,ADC_11db);
    in3=analogRead(PIN);
    
    if( in3 > 2599 ){
        ad3 = -1.457583e-7 * (float)in3 * (float)in3
            + 1.510116e-3 * (float)in3
            - 0.680858 + offset;
    }else{
        ad3 = 8.378998e-4 * (float)in3 + 8.158714e-2 + offset;
    }
    Serial.print("ADC (ATT=3;11dB) = ");
    Serial.print(ad3,3);
    Serial.print(" [V], ");
    Serial.println(in3);
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + offset;
        Serial.print("ADC (ATT=0; 0dB) = ");
        Serial.print(ad0,3);
        Serial.print(" [V], "); 
        Serial.println(in0);
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
