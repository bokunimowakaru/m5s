/*******************************************************************************
Example 02: Photo Frame for M5Stack
・Micro SDカードに保存したJPEG画像をLEDに表示します。
・本体LCD面のボタンで、以下の操作を行います。
    A: 最初の写真に戻る
    B: 押下中はスライドショーを止める
    C: 次の写真に進む

                                          Copyright (c) 2019-2020 Wataru KUNINO
********************************************************************************
【参考文献】
Arduino IDE 開発環境イントール方法：
https://github.com/m5stack/M5Stack/blob/master/docs/getting_started_ja.md
https://docs.m5stack.com/#/en/related_documents/Arduino_IDE

M5Stack Arduino Library API 情報：
https://docs.m5stack.com/#/ja/api
https://docs.m5stack.com/#/en/arduino/arduino_api
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリの組み込み
File fileRoot;                                  // SDカードのRootフォルダを定義

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.begin();                                 // M5Stack用ライブラリの起動
    M5.Lcd.fillScreen(BLACK);                   // LCDを消去
    M5.Lcd.println("Example 02 M5Stack Photo"); // LCDにタイトルを表示
    File fileRoot = SD.open("/");               // SDカードのRootフォルダを開く
    if( !fileRoot ){                            // 開けなかったとき
        M5.Lcd.print("SD not Ready");           // LCDにエラー表示
        return;                                 // setupを終了
    }
}

void loop(){                                    // 繰り返し実行する関数
    File file = fileRoot.openNextFile();        // 次のファイルを開く
    if( !file ){                                // 次のファイルが無かったとき
        fileRoot.close();                       // Rootフォルダを閉じる
        fileRoot = SD.open("/");                // Rootフォルダを開きなおす
        file = fileRoot.openNextFile();         // 最初のファイルを開く
    }
    String filename = file.name();              // ファイル名を取得する
    if( !filename.endsWith(".jpg") ) return;    // jpgでなければloopの先頭へ
    char filename_s[64];                        // 配列型の文字列変数を定義
    filename.toCharArray(filename_s, 64);       // ファイル名を代入する
    M5.Lcd.drawJpgFile(SD, filename_s);         // LCDにJPEGファイルを表示する
    M5.Lcd.setCursor(0,0);                      // 文字描画位置を左上に
    M5.Lcd.println(&filename_s[1]);             // LCDにファイル名を表示する
    
    for(int i = 0; i < 6000; i++){              // 6秒間の待ち時間処理
        if( M5.BtnA.read() ){                   // ボタンAが押されていた時
            fileRoot.close();                   // Rootフォルダを閉じる
            fileRoot = SD.open("/");            // Rootフォルダを開きなおす
            return;                             // loopの先頭へ戻る
        }
        while( M5.BtnB.read() ) delay(100);     // ボタンBが押下中に処理待ち
        if( M5.BtnC.read() ) return;            // loopの先頭へ戻る
        delay(1);                               // 1msの待ち時間処理
    }
}
