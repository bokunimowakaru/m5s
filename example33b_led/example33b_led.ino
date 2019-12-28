/*******************************************************************************
Example 33: ESP32 Wi-Fi インジケータ for M5Stack
LEDをWi-Fi経由で制御するワイヤレスLチカ実験を行います。

                                          Copyright (c) 2016-2020 Wataru KUNINO

【参考文献】
Arduino IDE 開発環境イントール方法：
https://github.com/m5stack/M5Stack/blob/master/docs/getting_started_ja.md
https://docs.m5stack.com/#/en/related_documents/Arduino_IDE

M5Stack Arduino Library API 情報：
https://docs.m5stack.com/#/ja/api
https://docs.m5stack.com/#/en/arduino/arduino_api
*******************************************************************************/

#include <M5Stack.h>                        // M5Stack用ライブラリの組み込み
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#define PIN_LED 22                          // GPIO 22
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define TIMEOUT 20000                       // タイムアウト 20秒
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
IPAddress ip = {0,0,0,0};                   // IPアドレス保持用の変数ipを定義
int led=0;                                  // LEDの状態

void lcd_cls(){                             // LCD表示
    M5.Lcd.fillScreen(BLACK);               // LCDを消去
    M5.Lcd.setCursor(0,0);                  // カーソル位置を左上に
    M5.Lcd.println("Example 32 M5Stack LED IO" + String(PIN_LED));
    if((uint32_t)ip == 0) return;
    M5.Lcd.print("http://");  M5.Lcd.print(ip); M5.Lcd.println("/");
}

void setup(){                               // 起動時に一度だけ実行する関数
    M5.Lcd.begin();                         // M5Stack用Lcdライブラリの起動
    lcd_cls();                              // LCDを消去
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    digitalWrite(PIN_LED,LOW);              // LEDを消灯
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    delay(10);                              // ESP32に必要な待ち時間
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        M5.Lcd.print(".");
    }
    ip = WiFi.localIP();                    // 本機のIPアドレスを変数ipへ代入
    server.begin();                         // サーバを起動する
    lcd_cls();
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[129];                            // 文字列変数を定義 129バイト128文字
    int i=0,len=0;                          // 文字列の長さカウント用の変数
    
    client = server.available();            // 接続されたクライアントを生成
    if(!client) return;                     // 非接続の時にloop()の先頭に戻る
    lcd_cls();
    M5.Lcd.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected() && i<TIMEOUT){ // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            M5.Lcd.write(c);                // 文字の内容をシリアルに出力(表示)
            switch(c){                      // 文字cに応じて
                case '\n':                  // 文字変数cの内容がLFのとき
                    if(strncmp(s,"GET /L",6)==0 && len>6){
                        if(s[6]=='0')led=0; // LEDをOFF
                        else led=1;         // LEDをON
                    }
                    i = TIMEOUT;            // whileループを抜ける
                    break;
                case '\0':                  // 文字変数cの内容が空のとき
                case '\r':                  // 文字変数cの内容がCRのとき
                    break;                  // 何もしない
                default:                    // その他の場合
                    if(len<128){            // 文字列変数の上限
                        s[len]=c;           // 文字列変数に文字cを追加
                        len++;              // 変数lenに1を加算
                        s[len]='\0';        // 文字列を終端
                    }
                    break;
            }
        }
        i++;                                // 変数iの値を1だけ増加させる
        delay(1);
    }
    client.println("HTTP/1.1 200 OK");  // HTTP OKを応答
    sprintf(s,
        "<p>LED=%01d\r\n<p><a href=\"/L1\">LED ON </a>,http://%d.%d.%d.%d/L1",
        led,ip[0],ip[1],ip[2],ip[3]);   // コンテンツ（の一部）を生成
    client.print("Content-Length: ");   // HTTPヘッダ情報を出力
    client.println(strlen(s)*2-10+65);  // コンテンツ長さを出力(改行2バイト)
    client.println();                   // HTTPヘッダの終了を出力
    client.println("<html>");           // HTML開始タグを出力(IE以外で必要)
    client.println("<meta name=\"viewport\" content=\"width=240\">");
    client.println(s);                  // HTTPコンテンツ(LED ON)を出力
    M5.Lcd.println(s);                  // シリアルへコンテンツを出力
    sprintf(s,
        "<p><a href=\"/L0\">LED OFF</a>,http://%d.%d.%d.%d/L0",
        ip[0],ip[1],ip[2],ip[3]);
    client.println(s);                  // HTTPコンテンツ(LED OFF)を出力
    M5.Lcd.println(s);                  // シリアルへコンテンツを出力
    client.println("</html>");          // HTML終了タグを出力(IE以外で必要)
    client.stop();                      // クライアントの切断
    M5.Lcd.println("Disconnected");     // シリアル出力表示
    digitalWrite(PIN_LED,led);          // LEDを制御
    M5.Lcd.invertDisplay(led);          // LCDの画面反転制御
}
