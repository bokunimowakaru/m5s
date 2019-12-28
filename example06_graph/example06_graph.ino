/*******************************************************************************
Example 06: ESP32 Wi-Fi LCD Graph表示版 for M5Stack
********************************************************************************

・各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示します。
・センサ値は液晶ディスプレイにグラフ表示します。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリの組み込み
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 受信ポート番号

WiFiUDP udp;                                    // UDP通信用のインスタンスを定義
char lcd[3][54];                                // 表示用変数を定義(3×40文字)
int lcd_n=0;                                    // LCD格納済の行数の変数定義

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.Lcd.begin();                             // M5Stack用Lcdライブラリの起動
    lineGraphInit(0, 40);                       // グラフ初期化(縦軸の範囲指定)
    M5.Lcd.println("Example 06 M5Stack Graph"); // LCDにタイトルを表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    M5.Lcd.setCursor(160,216);                  // テキスト文字表示位置を設定
    M5.Lcd.println(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
    udp.begin(PORT);                            // UDP通信御開始
}

void loop(){                                    // 繰り返し実行する関数
    int len,i;                                  // 文字列長を示す変数を定義
    len = udp.parsePacket();                    // 受信パケット長を変数lenに代入
    if(len==0)return;                           // 未受信のときはloop()の先頭へ
    memset(lcd[lcd_n], 0, 54);                  // 文字列変数lcdの初期化
    udp.read(lcd[lcd_n], 53);                   // 受信データを文字列変数lcdへ
    lineGraphPlot(atof(lcd[lcd_n]+8));          // 受信値をグラフ表示
    for(i = 0; i <= lcd_n; i++){                // 受信履歴表示 3行分
        int y = (27 + i) * 8;                   // 表示位置Y座標の計算
        M5.Lcd.fillRect(0, y, 320, 8, BLACK);   // 表示部の背景を塗る
        M5.Lcd.drawString(lcd[i], 0, y);        // 受信文字列を表示
    }
    lcd_n++;                                    // 次の行に更新
    if(lcd_n > 2){                              // 最終行を超えた時
        for(i=1;i<3;i++) strncpy(lcd[i-1],lcd[i],53);   // 1行ずつ繰り上げ
        lcd_n = 2;                              // 最終行を設定
    }
}
