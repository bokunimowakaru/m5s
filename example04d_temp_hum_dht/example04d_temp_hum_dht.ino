/*******************************************************************************
Example 04: ESP32 (IoTセンサ) Wi-Fi 温度計 Temprature & Humidity for M5Stack
********************************************************************************

・温湿度センサDHT12から取得した温度値と湿度値を送信するIoTセンサです。
・センサ値は液晶ディスプレイにアナログメータで表示します。
・WGBT(疑似)を計算しグラフに表示します。【追加機能】
・送信頻度を 約1分に1回に抑えました。【追加機能】

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリの組み込み
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include "lib_DHT12.h"                          // 温湿度センサDHT12用ライブラリ
#include <Wire.h>                               // 温湿度センサDHT12 I2C通信用

#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define DEVICE "humid_3,"                       // デバイス名(5字+"_"+番号+",")
IPAddress IP;                                   // ブロードキャストIP保存用
DHT12 dht12;                                    // 温湿度センサDHT12用

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.Lcd.begin();                             // M5Stack用Lcdライブラリの起動
    M5.Lcd.setBrightness(31);                   // 輝度を下げる（省エネ化）
    Wire.begin();                               // I2C通信用ライブラリの起動
    analogMeterInit();                          // アナログメータの初期化
    lineGraphInit(13, 33);                       // グラフ初期化(縦軸の範囲指定)
          // (18℃-5℃)～(28℃+5℃)
    M5.Lcd.println("Example 04 M5Stack Temp & Hum (DHT12)"); // タイトル表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    IP = WiFi.localIP();                        // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    M5.Lcd.println(IP);                         // UDP送信先IPアドレスを表示
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    float temp,hum,wgbt;                        // 温度値、湿度値用の変数
    
    temp = dht12.readTemperature();             // 温度値の取得
    hum = dht12.readHumidity();                 // 湿度値の取得
    wgbt = 0.725 * temp + 0.0368 * hum + 0.00364 * temp * hum - 3.246 + 0.5;
    analogMeterNeedle(0,temp);                  // メータへ表示
    analogMeterNeedle(1,hum);                   // メータへ表示
    lineGraphPlot(wgbt);                        // WGBTをグラフ表示
    udp.beginPacket(IP, PORT);                  // UDP送信先を設定
    udp.print(DEVICE);                          // デバイス名を送信
    udp.print(temp,1);                          // 温度値を送信
    udp.print(", ");                            // カンマを送信
    udp.println(hum,1);                         // 湿度値を送信
    udp.endPacket();                            // UDP送信の終了(実際に送信する)
    if(11. < wgbt && wgbt < 31.){
        M5.Lcd.fillRect(0, 202, 320, 38, BLACK);    // 表示部の背景を塗る
    }else{
        M5.Lcd.fillRect(0, 202, 320, 38, TFT_RED);  // 表示部の背景を塗る
        M5.Speaker.tone(440);                       // スピーカ出力 440Hzを出力
        delay(100);                                 // 100msの待ち時間処理
        M5.Speaker.end();                           // スピーカ出力を停止する
    }
    String S="WGBT= "+String(wgbt,1)+"C ("+String(temp,1)+"C, "+String(hum,0)+"%)";
    M5.Lcd.drawCentreString(S, 160, 210, 4);    // 受信文字列を表示
    delay(110);                                 // 110ミリ秒間の待ち時間処理

    // 待機処理 //
    WiFi.mode(WIFI_OFF);                        // 無線LANをOFFモードに設定
    delay(55000);                               // 55秒間の待ち時間処理
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    M5.Lcd.setCursor(0, 202);
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    IP = WiFi.localIP();                        // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
}
