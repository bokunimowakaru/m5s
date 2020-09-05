/*******************************************************************************
Example 04: ESP32 (IoTセンサ) Wi-Fi 温度計 Temprature & Humidity for M5Stack
********************************************************************************

・温湿度センサSHT30から取得した温度値と湿度値を送信するIoTセンサです。
・センサ値は液晶ディスプレイにアナログメータで表示します。

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリの組み込み
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include <Wire.h>                               // 温湿度センサSHT30 I2C通信用

#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define DEVICE "humid_3,"                       // デバイス名(5字+"_"+番号+",")
IPAddress IP;                                   // ブロードキャストIP保存用

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.Lcd.begin();                             // M5Stack用Lcdライブラリの起動
    Wire.begin();                               // I2C通信用ライブラリの起動
    analogMeterInit();                          // アナログメータの初期化
    M5.Lcd.println("Example 04 M5Stack Temp & Hum"); // LCDにタイトルを表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    IP = WiFi.localIP();                        // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    M5.Lcd.println(IP);                         // UDP送信先IPアドレスを表示
    i2c_sht30_getStat();
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    float temp,hum;                             // 温度値、湿度値用の変数
    
    temp = i2c_sht30_getTemp();                 // 温度値の取得
    hum = i2c_sht30_getHum();                   // 湿度値の取得
    analogMeterNeedle(0,temp);                  // メータへ表示
    analogMeterNeedle(1,hum);                   // メータへ表示
    udp.beginPacket(IP, PORT);                  // UDP送信先を設定
    udp.print(DEVICE);                          // デバイス名を送信
    udp.print(temp,1);                          // 温度値を送信
    udp.print(", ");                            // カンマを送信
    udp.println(hum,1);                         // 湿度値を送信
    M5.Lcd.print(temp,1);                       // シリアル出力表示
    M5.Lcd.print(',');                          // シリアル出力表示
    M5.Lcd.print(hum,1);                        // シリアル出力表示
    M5.Lcd.print(' ');                          // シリアル出力表示
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    delay(6000);                                // 6秒間の待ち時間処理
}
