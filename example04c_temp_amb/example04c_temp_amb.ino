/*******************************************************************************
Example 04: ESP32 (IoTセンサ) Wi-Fi 温度計 Temprature for M5Stack [Ambient対応]
********************************************************************************

・ESP32マイコン内蔵の温度センサから取得した温度値を送信するIoTセンサです。
・センサ値は液晶ディスプレイにアナログメータで表示します。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリの組み込み
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include "lib_Ambient.h"                        // Ambient通信用ライブラリ
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define Ambient_channelId 100                   // AmbientのチャネルID 
#define Ambient_writeKey "0123456789abcdef"     // Ambientのライトキー 
#define Ambient_d 1                             // Ambientの送信データ番号 
#define PORT 1024                               // 送信のポート番号
#define DEVICE "temp0_3,"                       // デバイス名(5字+"_"+番号+",")
#define TEMP_ADJ -25.0                          // 温度値の補正用
IPAddress IP;                                   // ブロードキャストIP保存用
WiFiClient client;                              // Ambient送信用WiFiクライアント
Ambient ambient;                                // Ambient送信用インスタンス生成

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.Lcd.begin();                             // M5Stack用Lcdライブラリの起動
    analogMeterInit("Temp.C", 0, 40);           // アナログメータの初期化
    M5.Lcd.println("Example 04 M5Stack Temprature"); // LCDにタイトルを表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    IP = WiFi.localIP();                        // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    M5.Lcd.println(IP);                         // UDP送信先IPアドレスを表示
    ambient.begin(Ambient_channelId, Ambient_writeKey, &client);// Ambient開始
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    float temp;                                 // 温度値用の変数
    
    temp = temperatureRead() + TEMP_ADJ;        // 温度値の取得
    analogMeterNeedle(temp);                    // メータへ表示
    udp.beginPacket(IP, PORT);                  // UDP送信先を設定
    udp.print(DEVICE);                          // デバイス名を送信
    udp.println(temp,1);                        // 温度値を送信
    M5.Lcd.print(temp,1);                       // シリアル出力表示
    M5.Lcd.print(' ');                          // シリアル出力表示
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    if(millis() % 30000ul >= 9000){             // 30秒が経過したかどうかの判定
        delay(6000);                            // 6秒間の待ち時間処理(更新間隔)
        return;                                 // 繰り返し処理の再開
    }                                           // 以下は30秒に1回だけの処理
    ambient.set(Ambient_d,String(temp).c_str());// 変数tempの値をAmbientへ送信
    ambient.send();                             // Ambient送信終了(実際に送信)
    M5.Lcd.print("Amb ");                       // シリアル出力表示
    delay(9000);                                // 9秒間の待ち時間(重複送信防止)
}
