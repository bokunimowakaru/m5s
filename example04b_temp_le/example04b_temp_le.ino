/*******************************************************************************
Example 04: ESP32 (IoTセンサ) Wi-Fi 温度計 Temprature [LCDなし・低消費電力]
********************************************************************************

・ESP32マイコン内蔵の温度センサから取得した温度値を送信するIoTセンサです。
・乾電池などで動作するIoTセンサ用の基本形です。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include "esp_sleep.h"                          // ESP32用Deep Sleep ライブラリ
#define SSID "1234ABCD"                         // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define SLEEP_P 55*1000000ul                    // スリープ時間 55秒(uint32_t)
#define DEVICE "temp0_3,"                       // デバイス名(5文字+"_"+番号+",")
#define TEMP_ADJ -25.0                          // 温度値の補正用
IPAddress IP;

void setup(){                                   // 起動時に一度だけ実行する関数
    Serial.begin(115200);                       // 動作確認のためのシリアル出力
    Serial.println("ESP32 04 LE");              // 「ESP32 04 LE」をシリアル表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    delay(10);                                  // ESP32に必要な待ち時間
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        Serial.print(".");
    }
    IP = WiFi.localIP();                        // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    Serial.println(IP);                         // UDP送信先IPアドレスを表示
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    float temp;                                 // 温度値用の変数
    
    temp = temperatureRead() + TEMP_ADJ;        // 温度値の取得
    udp.beginPacket(IP, PORT);                  // UDP送信先を設定
    udp.print(DEVICE);                          // デバイス名を送信
    udp.println(temp,1);                        // 温度値を送信
    Serial.println(temp,1);                     // シリアル出力表示
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    delay(200);                                 // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                    // Deep Sleepモードへ移行
}
