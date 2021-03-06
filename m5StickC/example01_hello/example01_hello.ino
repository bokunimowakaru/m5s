/*******************************************************************************
Example 01: Hello, world! for M5StickC
・起動時にLEDを点滅、LCDにタイトルを表示します。
・本体のボタンを押すと、ボタンに応じてメッセージを表示します。

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

#include <M5StickC.h>                           // M5StickC用ライブラリ

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(M5_LED,OUTPUT);                     // LEDのIOを出力に設定
    digitalWrite(M5_LED,LOW);                   // LED ON
    delay(100);                                 // 100msの待ち時間処理
    digitalWrite(M5_LED,HIGH);                  // LED OFF
    
    M5.begin();                                 // M5Stack用ライブラリの起動
    M5.Lcd.fillScreen(BLACK);                   // LCDを消去
    M5.Axp.ScreenBreath(7+2);                   // LCDの輝度を2に設定
    M5.Lcd.setRotation(1);                      // LCDを横向き表示に設定
    M5.Lcd.println("Example 01 M5StickC LCD");  // LCDにタイトルを表示
}

void loop(){                                    // 繰り返し実行する関数
    M5.BtnA.read();                             // ボタンAの状態を確認
    M5.BtnB.read();                             // ボタンBの状態を確認
    
    int btnA = M5.BtnA.wasPressed();            // ボタンAの状態を変数btnAへ代入
    int btnB = M5.BtnB.wasPressed();            // ボタンBの状態を変数btnAへ代入
    
    if( btnA == 1 ){                            // ボタンAが押されていた時
        M5.Lcd.println("Hello, world!");        // LCDへメッセージを表示
    }
    if( btnB == 1 ){                            // ボタンBが押されていた時
        M5.Lcd.println("IoT Device M5StickC");  // LCDへメッセージを表示
    }
    if( M5.Axp.GetBtnPress() ){                 // 電源が押された時
        M5.Lcd.fillScreen(BLACK);               // LCDを消去
        M5.Lcd.setCursor(0,0);                  // 文字描画位置を画面左上へ
        M5.Lcd.println("Screen Cleared");       // LCDへメッセージを表示
    }
}
