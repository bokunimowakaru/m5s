/*******************************************************************************
Example 07: ESP32 Wi-Fi LCD Logger for M5Stack
********************************************************************************

・各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示します。
・TF Cardカードスロットへ装着した Micro SD カードへログを保存します。
・IoT温湿度センサのセンサ値を液晶ディスプレイにメーター＆グラフ表示します。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリの組み込み
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 受信ポート番号
#define NTP_SERVER "ntp.nict.jp"                // NTPサーバのURL
#define NTP_PORT 8888                           // NTP待ち受けポート

WiFiUDP udp;                                    // UDP通信用のインスタンスを定義
char lcd[4][54];                                // 表示用変数を定義(3×40文字)
int lcd_n=0;                                    // LCD格納済の行数の変数定義
unsigned long time_now=0;                       // 1970年～millis()=0までの秒数

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.begin();                                 // M5Stack用ライブラリの起動
    analogMeterInit();                          // アナログメータの初期化
    lineGraphInit(0, 40);                       // グラフ初期化(縦軸の範囲指定)
    M5.Lcd.println("Example 07 M5Stack Logger");// LCDにタイトルを表示
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        M5.Lcd.print('.');                      // 進捗表示
    }
    M5.Lcd.setCursor(160, 110 + 84);            // テキスト文字表示位置を設定
    M5.Lcd.println(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
    udp.begin(PORT);                            // UDP通信御開始
}

void loop(){                                    // 繰り返し実行する関数
    
    /* NTPサーバから時刻情報を受信 */
    unsigned long time = millis();              // ミリ秒の取得
    if(time < 9000){                            // クロックのリセットを検出
        time_now = getNTP(NTP_SERVER,NTP_PORT); // NTPを用いて時刻を取得
        if(time_now) time_now -= millis()/1000; // 経過時間を減算
        while(millis() < 9000) delay(1);        // 重複実行防止
    }
    
    /* ioTセンサからのデータを受信 */
    int len,i;                                  // 文字列長を示す変数を定義
    char rx[65];                                // 受信データ保持用変数を定義
    len = udp.parsePacket();                    // 受信パケット長を変数lenに代入
    if(len == 0) return;                        // 未受信のときはloop()の先頭へ
    memset(rx, 0, 65);                          // 受信データ用変数rxの初期化
    udp.read(rx, 64);                           // 受信データを変数rxへ
    udp.flush();                                // UDP受信バッファを破棄する
    for(i=8;i<64;i++) if(rx[i]<32) rx[i]='\0';  // 特殊文字を削除
    if(strlen(rx) < 8) return;                  // 文字数不足でloopの先頭へ戻る
    if(rx[5] != '_' || rx[7] != ',') return;    // 規則不一致でloopの先頭へ戻る

    /* 受信データのテキスト表示(画面下部) */
    char date[20];                              // 日付データ格納用
    time2txt(date, time_now + time / 1000);     // 日時をテキストに変換する
    memset(lcd[lcd_n], 0, 54);                  // 文字列変数lcdの初期化
    strcpy(lcd[lcd_n], date);                   // 日付をLCD表示用変数へコピー
    lcd[lcd_n][19] = ' ';                       // 日付の後ろに空白文字を追加
    strncpy(lcd[lcd_n] + 20, rx, 33);           // 受信データをLCD表示用変数へ
    for(i = 0; i <= lcd_n; i++){                // 受信履歴表示 3行分
        int y = 110 + 84 + 8 + i * 8;           // 表示位置Y座標の計算
        M5.Lcd.fillRect(0, y, 320, 8, BLACK);   // 表示部の背景を塗る
        M5.Lcd.drawString(lcd[i], 0, y);        // 受信文字列を表示
    }
    
    /* Wi-Fi温度センサからの受信処理 */
    if( !strncmp(rx,"temp",4) ){                // デバイス名の先頭4文字がtemp
        lineGraphPlot(atof(rx + 8));            // 受信値(温度)をグラフ表示
        analogMeterNeedle(0,atof(rx + 8));      // メータ(左)へ表示
    }

    /* Wi-Fi温湿度センサ、Wi-Fi環境センサからの受信処理 */
    if( !strncmp(rx,"humid",5) || !strncmp(rx,"envir",5)){
        lineGraphPlot(atof(rx + 8));            // 受信値(温度)をグラフ表示
        analogMeterNeedle(0,atof(rx + 8));      // 9文字目からの値をメータ(左)へ
        char *p = strstr(rx + 8,",");           // カンマ「,」を検索
        if(p){                                  // カンマが存在したとき
            analogMeterNeedle(1,atof(p+1));     // 次文字以降の値をメータ(右)へ
        }
    }

    /* TFカードスロットへ挿入した Micro SDカードへの保存 */
    File file;                                  // ファイルアクセス用の実体
    char filename[17] = "/log_";                // ファイル名保存用変数
    strncpy(filename + 5, rx, 7);               // 受信したデバイス名を代入
    strcpy(filename + 5 + 7, ".csv");           // ファイル名に拡張子を追加
    file = SD.open(filename, "a");              // 追記保存するファイルを開く
    if(file){                                   // ファイルが開けた場合
        file.print(date);                       // 日時を出力する
        file.println(rx + 7);                   // 受信データをファイル出力
        file.close();                           // ファイルを閉じる
    }
    
    lcd_n++;                                    // 次の行に更新
    if(lcd_n > 3){                              // 最終行を超えた時
        for(i=1;i<4;i++) strncpy(lcd[i-1],lcd[i],53);   // 1行ずつ繰り上げ
        lcd_n = 3;                              // 最終行を設定
    }
}
