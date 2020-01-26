/*******************************************************************************
Example 36(=32+4): M5StickC ケチケチ運転術

・パソコンなどの連続使用時間をカウントし、その値を送信します。
・ゲームやテレビの使用時間を表示＆Wi-Fi通知する端末です。

USBから給電中は通電継続時間(上段)と、それまでのスリープ時間(下段)を表示します。
また、1分ごとに以下のようなメッセージをWi-Fi送信します。
USB給電を止めたときもWi-Fi送信します。

                デバイス,分,USB,電圧,電流
2020/01/20 22:01, count_1,1, 1, 4181, 34
2020/01/20 22:02, count_1,2, 1, 4117, 0
2020/01/20 22:03, count_1,3, 1, 4117, 0
2020/01/20 22:03, count_1,3, 0, 4117, -29

バッテリ駆動中は間欠動作を行い、USB給電を待ち受けます。
待ち受け中、M5ボタンを押すと、前回の通電時間(上段)とスリープ継続時間(下段)を
表示します。約3秒で表示は消えます。

間欠動作中に、USBから電源を供給すると復帰します。（10秒に1度だけ確認）

                                          Copyright (c) 2016-2020 Wataru KUNINO
*******************************************************************************/

#include <M5StickC.h>                           // M5StickC用ライブラリ
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 送信のポート番号
#define SLEEP_P 10*1000000ul                    // スリープ時間 10秒(uint32_t)
#define DEVICE "count_1,"                       // デバイス名(5字+"_"+番号+",")

RTC_DATA_ATTR int WAKE_DUR = 0;                 // 前回、起動していた合計時間
RTC_DATA_ATTR uint32_t SLEEP_DUR = 0;           // スリープしていた合計時間
int wake;                                       // 起動理由

void setup(){                                   // 起動時に一度だけ実行する関数
    wake = TimerWakeUp_init();
    int mv = M5.Axp.GetVusbinData()* 1.7f;
    M5.Axp.begin();
    if( wake != 0 ) SLEEP_DUR += SLEEP_P + millis();
    if((wake == 3 || wake == 4) && mv < 3300) sleep();
    pinMode(M5_LED,OUTPUT);                     // LEDのIOを出力に設定
    M5.begin();                                 // M5StickC用Lcdライブラリの起動
    M5.Axp.ScreenBreath(7+1);                   // LCDの輝度を1に設定
    M5.Lcd.setRotation(1);                      // LCDを横向き表示に設定
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
}

void loop() {
    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    IPAddress IP = WiFi.localIP();              // IPアドレスを取得
    IP[3] = 255;                                // ブロードキャストアドレスに
    
    int stat = (int8_t)((byte)WiFi.status());
    int bvus_mV = M5.Axp.GetVusbinData()* 1.7f; // USB電圧を取得
    int batt_mV = M5.Axp.GetVbatData() * 1.1f;  // 電池電圧を取得
    int batt_mA = M5.Axp.GetIchargeData()/2
                - M5.Axp.GetIdischargeData()/2; // 充電放電電流を取得
    int time = (int)(millis() / 1000ul);
    int time2 = (int)(SLEEP_DUR / 1000ul);
    int usb = (bvus_mV > 3300);                 // USB接続状態フラグ
    
    M5.Lcd.setTextSize(1);                      // 文字表示サイズを1倍に設定
    M5.Lcd.setCursor(0,0);                      // 文字描画位置を画面左上へ
    M5.Lcd.printf("WiFi=%d ",stat);
    M5.Lcd.printf("Usb=%1.2f \n",bvus_mV/1000.);
//  Serial.printf("USB %1.2fV\n",bvus_mV/1000.);
    M5.Lcd.printf("Batt %1.2fV ",batt_mV/1000.);
    M5.Lcd.printf("%dmA  \n\n",batt_mA);
    M5.Lcd.setTextSize(3);                      // 文字表示サイズを3倍に設定
    M5.Lcd.setCursor(9,24);
    if(usb) M5.Lcd.printf("%02d:%02d:%02d\n",time/3600,(time/60)%60,time%60);
    else M5.Lcd.printf("%02d:%02d:%02d\n",WAKE_DUR/3600,(WAKE_DUR/60)%60,WAKE_DUR%60);
    M5.Lcd.setCursor(9,48);
    M5.Lcd.printf("%02d:%02d:%02d\n",time2/3600,(time2/60)%60,time2%60);
    
    if(stat == WL_CONNECTED && (time % 60 == 0) || !usb ){
        udp.beginPacket(IP, PORT);              // UDP送信先を設定
        udp.print(DEVICE);                      // デバイス名を送信
        udp.print((int)(time/60));              // 変数timeを送信
        udp.print(", ");                        // カンマを送信
        udp.print(usb);                         // 変数USB状態を送信
        udp.print(", ");                        // カンマを送信
        udp.print(batt_mV);                     // 変数batt_mVの値を送信
        udp.print(", ");                        // カンマを送信
        udp.println(batt_mA);                   // 変数batt_mAの値を送信
        udp.endPacket();                        // UDP送信の終了(実際に送信する)
        delay(200);                             // 送信待ち時間
    }
    if(!usb){
        if(wake == 1 || wake == 2){
            delay(3000);
        }else{
            WAKE_DUR = time;
            TimerWakeUp_setBootCount(0);
        }
        sleep();                                // USB電源供給無し時にsleep
    }
    while (millis()%1000 != 0) delayMicroseconds(900);
    digitalWrite(M5_LED,LOW); delay(10);        // LEDの点灯
    digitalWrite(M5_LED,HIGH);                  // LEDの消灯
}

void sleep(){
    while(M5.BtnA.read()) delay(1);
    M5.Axp.ScreenBreath(0);
    M5.Lcd.fillScreen(BLACK);
//  M5.Lcd.sleep();                             // CPP未実装 M5StickC Ver.0.1.1
//  M5.Axp.LightSleep(SLEEP_P);                 // HOMEキーで起動せず Ver.0.1.1
//  M5.Axp.DeepSleep(SLEEP_P);                  // HOMEキーで起動せず Ver.0.1.1

    // RTCクロックを停止 Wire1 0x51 削減効果見えず
    Wire1.beginTransmission(0x51);
    Wire1.write(0x10);                          // PCF8563 RTC clock is stopped
    Wire1.write(0x00);
    Wire1.endTransmission();
    
    // LCDコントローラをスリープ 1.2 mA削減
    bool swap = M5.Lcd.getSwapBytes();
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.writecommand(ST7735_SLPIN);
    M5.Lcd.setSwapBytes(swap);
    
    // LCDバックライト用電源をOFF
    M5.Axp.SetLDO2(false);                      // LCDバックライト用電源
    
    pinMode(BUTTON_A_PIN,INPUT_PULLUP);
    while(digitalRead(BUTTON_A_PIN) == LOW) delay(1);
    TimerWakeUp_setExternalInput((gpio_num_t)BUTTON_A_PIN, LOW);
    TimerWakeUp_setSleepTime((int)(SLEEP_P/1000000ul));
    TimerWakeUp_sleep();
}
