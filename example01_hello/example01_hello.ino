/*******************************************************************************
Example 01: Hello, world! for M5Stack
・起動時にスピーカから音を鳴らし、LCDにタイトルを表示します。
・本体上面のボタンを押すと、ボタンに応じてメッセージを表示します。

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

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.Speaker.begin();                         // M5Stack用スピーカの起動
    M5.Speaker.tone(880);                       // スピーカ出力 880Hzの音を出力
    delay(100);                                 // 100msの待ち時間処理
    M5.Speaker.end();                           // スピーカ出力を停止する
    
    M5.Lcd.begin();                             // M5Stack用Lcdライブラリの起動
    M5.Lcd.fillScreen(BLACK);                   // LCDを消去
    M5.Lcd.setTextSize(2);                      // 文字表示サイズを2倍に設定
    M5.Lcd.println("Example 01 M5Stack LCD");   // LCDにタイトルを表示
}

void loop(){                                    // 繰り返し実行する関数
    M5.BtnA.read();                             // ボタンAの状態を確認
    M5.BtnB.read();                             // ボタンBの状態を確認
    M5.BtnC.read();                             // ボタンCの状態を確認
    
    int btnA = M5.BtnA.wasPressed();            // ボタンAの状態を変数btnAへ代入
    int btnB = M5.BtnB.wasPressed();            // ボタンBの状態を変数btnAへ代入
    int btnC = M5.BtnC.wasPressed();            // ボタンCの状態を変数btnCへ代入
    
    if( btnA == 1 ){                            // ボタンAが押されていた時
        M5.Lcd.println("Hello, world!");        // LCDへメッセージを表示
    }
    if( btnB == 1 ){                            // ボタンBが押されていた時
        M5.Lcd.println("IoT Device M5Stack");   // LCDへメッセージを表示
    }
    if( btnC == 1 ){                            // ボタンCが押されていた時
        M5.Lcd.fillScreen(BLACK);               // LCDを消去
        M5.Lcd.setCursor(0,0);                  // 文字描画位置を画面左上へ
        M5.Lcd.println("Screen Cleared");       // LCDへメッセージを表示
    }
}
