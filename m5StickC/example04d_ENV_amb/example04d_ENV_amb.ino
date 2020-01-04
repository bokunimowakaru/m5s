/*******************************************************************************
Example 04: ESP32 Wi-Fi 環境センサ for M5StickC + ENV [Ambient送信機能つき]
********************************************************************************

・純正オプション ENV HAT から取得した温度値と湿度値、気圧値を送信するIoTセンサ
・センサ値は液晶ディスプレイにアナログメータで表示します。
・本体のM5ボタン（ホームボタン）を押すと湿度、気圧、電池電圧に切り替わります。
・6秒後にスリープ状態に遷移し、30秒後、スリープから復帰します。
・スリープ中にM5ボタン（ホームボタン）を押すと復帰します。

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5StickC.h>                           // M5StickC用ライブラリ
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include "esp_sleep.h"                          // ESP32用Deep Sleep ライブラリ
#include "lib_DHT12.h"                          // 温湿度センサDHT12用ライブラリ
#include <Wire.h>                               // 温湿度センサDHT12 I2C通信用
#include "lib_Ambient.h"                        // Ambient通信用ライブラリ

#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define Ambient_channelId 100                   // AmbientのチャネルID 
#define Ambient_writeKey "0123456789abcdef"     // Ambientのライトキー 
#define Ambient_d 1                             // Ambientの送信データ番号 
#define PORT 1024                               // 送信のポート番号
#define DEVICE "envir_5,"                       // デバイス名(5字+"_"+番号+",")
#define SLEEP_P 30*1000000ul                    // スリープ時間 30秒(uint32_t)

IPAddress IP;                                   // ブロードキャストIP保存用
DHT12 dht12;                                    // 温湿度センサDHT12用
RTC_DATA_ATTR int disp = 0;                     // メータ表示番号 0～

void disp_init(){
    if(disp == 0){
        analogMeterInit("Celsius", "Temp.", 0, 40);
    }else if(disp == 1){
        analogMeterInit("RH%", "Humi.", 0, 100);
    }else if(disp == 2){
        analogMeterInit("hPa", "Pres.", 1013-26, 1013+26);
    }else if(disp == 3){
        analogMeterInit("mV", "Batt.", 3000, 5000);
    }else if(disp == 4){
        analogMeterInit("mA", "Batt.I", 0, 160);
    }
}

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(M5_LED,OUTPUT);
    digitalWrite(M5_LED,LOW);                   // LED ON
    M5.begin();                                 // M5StickC用Lcdライブラリの起動
    TimerWakeUp_init();
    i2c_bme280_Setup(0,26);                     // I2C通信用ライブラリの起動
    M5.Axp.ScreenBreath(7+2);
    M5.Lcd.setRotation(1);
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    disp_init();                                // アナログメータ表示の初期化
    digitalWrite(M5_LED,HIGH);                  // LED OFF
}

void loop(){                                    // 繰り返し実行する関数
    WiFiUDP udp;                                // UDP通信用のインスタンス定義
    Ambient ambient;                            // Ambient送信用インスタンス生成
    WiFiClient client;                          // Ambient送信用WiFiクライアント
    float temp,temp1,hum,temp2,press,batt;      // 温度値、湿度値用の変数
    
    for(int i=0; i<6000; i++){                  // 6秒間の処理
        if( i % 500 == 0){
            M5.Axp.ScreenBreath(7 + 2 - i / 3000);
            temp1 = dht12.readTemperature();            // 温度値の取得
            hum = dht12.readHumidity();                 // 湿度値の取得
            temp2 = i2c_bme280_getTemp();
            press = i2c_bme280_getPress();
            temp = (temp1 + temp2) / 2;
            batt = (float)M5.Axp.GetVbatData() * 1.1;
            Serial.printf("(%.2f,%.2f)",temp1,temp2);   // 温度値をシリアル出力
            if(disp==0){
                analogMeterNeedle(temp,5);
            }else if(disp == 1){
                analogMeterNeedle(hum,5);
            }else if(disp == 2){
                analogMeterNeedle(press,5);
            }else if(disp == 3){
                analogMeterNeedle(batt, 5);
            }else if(disp == 4){
                analogMeterNeedle((float)M5.Axp.GetIdischargeData(), 5);
            }
            M5.Lcd.setTextColor(BLACK,WHITE);
            M5.Lcd.setCursor(0,0);
            int stat = WiFi.status();
            if( stat == WL_CONNECTED) M5.Lcd.printf("(@)");
            else M5.Lcd.printf("(%d)",stat);
            for(int j=6000;j>i;j-=1000)M5.Lcd.print('.'); M5.Lcd.print(' ');
            Serial.print(temp,1);               // 温度値をシリアル出力
            Serial.print(", ");                 // カンマをシリアル出力
            Serial.print(hum,1);                // 湿度値をシリアル出力
            Serial.print(", ");                 // カンマをシリアル出力
            Serial.print(press,1);              // 気圧値をシリアル出力
            Serial.print(", ");                 // カンマをシリアル出力
            Serial.println(batt,0);             // 電圧値をシリアル出力
            while(M5.BtnA.read()) delay(10);
        }
        M5.BtnA.read();
        if(M5.BtnA.wasPressed()){
            M5.Axp.ScreenBreath(7+2);
            i = -1;
            disp++;
            if(disp>4) disp=0;
            disp_init();                        // アナログメータ表示の初期化
        }
        delay(1);
    }
    if(WiFi.status() == WL_CONNECTED){
        IP = WiFi.localIP();                    // IPアドレスを取得
        IP[3] = 255;                            // ブロードキャストアドレスに
        udp.beginPacket(IP, PORT);              // UDP送信先を設定
        udp.print(DEVICE);                      // デバイス名を送信
        udp.print(temp,1);                      // 温度値を送信
        udp.print(", ");                        // カンマを送信
        udp.print(hum,1);                       // 湿度値を送信
        udp.print(", ");                        // カンマを送信
        udp.print(press,1);                     // 気圧値を送信
        udp.print(", ");                        // カンマを送信
        udp.println(batt,0);                    // 電圧値を送信
        udp.endPacket();                        // UDP送信の終了(実際に送信する)
        ambient.begin(Ambient_channelId, Ambient_writeKey, &client);// Ambient
        ambient.set(Ambient_d,String(temp).c_str());	// tempの値をAmbientへ
        ambient.set(Ambient_d+1,String(hum).c_str());	// humの値をAmbientへ
        ambient.set(Ambient_d+2,String(press).c_str());	// pressの値をAmbientへ
        ambient.set(Ambient_d+3,String(batt).c_str());	// battの値をAmbientへ
        ambient.send();                             	// Ambientへ実際に送信
    }
    M5.Axp.ScreenBreath(0);
    M5.Lcd.fillScreen(BLACK);
//  M5.Lcd.sleep();                             // 未実装
//  M5.Axp.LightSleep(SLEEP_P);                 // HOMEキーで起動せず Ver.0.1.1
//  M5.Axp.DeepSleep(SLEEP_P);                  // HOMEキーで起動せず Ver.0.1.1
    bool swap = M5.Lcd.getSwapBytes();
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.writecommand(ST7735_SLPIN);
    M5.Lcd.setSwapBytes(swap);
    delay(200);                                 // 送信待ち時間
    M5.Axp.SetLDO2(false);                      // LCDバックライト用電源
//  M5.Axp.SetLDO3(false);                      // LCDロジック電源 0.1.1未対応
    pinMode(BUTTON_A_PIN,INPUT_PULLUP);
    while(digitalRead(BUTTON_A_PIN) == LOW);
    TimerWakeUp_setExternalInput((gpio_num_t)BUTTON_A_PIN, LOW);
    TimerWakeUp_setSleepTime((int)(SLEEP_P/1000000ul));
    TimerWakeUp_sleep();
}
