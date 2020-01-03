/*******************************************************************************
Example 04: ESP32 (IoTセンサ) Wi-Fi 温度計 Temprature & Humidity for M5StickC
********************************************************************************

・温湿度センサDHT12から取得した温度値と湿度値を送信するIoTセンサです。
・センサ値は液晶ディスプレイにアナログメータで表示します。

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5StickC.h>                           // M5StickC用ライブラリ
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include "lib_DHT12.h"                          // 温湿度センサDHT12用ライブラリ
#include <Wire.h>                               // 温湿度センサDHT12 I2C通信用

#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define DEVICE "humid_5,"                       // デバイス名(5字+"_"+番号+",")
IPAddress IP;                                   // ブロードキャストIP保存用
DHT12 dht12;                                    // 温湿度センサDHT12用
int disp = 0;

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.begin();                                 // M5StickC用Lcdライブラリの起動
    Wire.begin(0,26);                           // I2C通信用ライブラリの起動
    Serial.begin(115200);                       // 動作確認のためのシリアル出力開始
    M5.Axp.ScreenBreath(7+2);
    M5.Lcd.setRotation(1);
    M5.Lcd.println("Example 04 Temp & Hum");    // LCDにタイトルを表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    IP = WiFi.localIP();                        // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    M5.Lcd.println(IP);                         // UDP送信先IPアドレスを表示
    Serial.println(IP);                         // UDP送信先IPアドレスを表示
//  while(!M5.BtnA.wasPressed())M5.BtnA.read(); // ボタンAの押下待ち
    delay(3000);
    analogMeterInit();                          // アナログメータの初期化
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    float temp,hum;                             // 温度値、湿度値用の変数
    
    temp = dht12.readTemperature();             // 温度値の取得
    hum = dht12.readHumidity();                 // 湿度値の取得
    if(disp==0){
        analogMeterNeedle(temp,5);
    }else if(disp==1){
        analogMeterNeedle(hum,5);
    }
    udp.beginPacket(IP, PORT);                  // UDP送信先を設定
    udp.print(DEVICE);                          // デバイス名を送信
    udp.print(temp,1);                          // 温度値を送信
    udp.print(", ");                            // カンマを送信
    udp.println(hum,1);                         // 湿度値を送信
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    Serial.print(temp,1);                       // 温度値を送信
    Serial.print(", ");                         // カンマを送信
    Serial.println(hum,1);                      // 湿度値を送信
    for(int i=0; i<6000; i++){                  // 6秒間の待ち時間処理
        M5.BtnA.read();
        if(M5.BtnA.wasPressed()){
            disp++;
            if(disp>1) disp=0;
            if(disp==0){
                analogMeterInit("Celsius", "Temp.", 0, 40);
                analogMeterNeedle(temp,5);
            }else if (disp==1){
                analogMeterInit("RH%", "Humi.", 0, 100);
                analogMeterNeedle(hum,5);
            }
        }
        delay(1);
    }
}
