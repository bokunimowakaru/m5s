/*******************************************************************************
Example 26: TFTPクライアント搭載 テキスト ビューワ for M5StickC

・TFTPサーバから取得したコンテンツ（テキストファイル）をM5StickCで表示します。
・M5ボタン（ボタンA）長押しでTFTPサーバへのアクセスを行います。

【バッテリ動作】
・バッテリ動作中はLCD画面が6秒後に消え、スリープ動作に移行します

【スリープ中】
・M5ボタン（ボタンA）長押しで起動し、TFTPサーバから取得したデータを表示します。
・M5ボタン（ボタンA）短押しで起動し、以前に取得したデータを表示します。

【USB電源供給時の動作】
・USB電源供給中はLCD画面に表示を行い続けます。
・USB電源から切断すると、6秒後にスリープ動作に移行します。

                                          Copyright (c) 2016-2020 Wataru KUNINO
********************************************************************************

TFTPサーバ上からＴＸＴファイルをダウンロードし、ＬＣＤに表示します。

TFTPとは
　TFTPはネットワーク機器などの設定ファイルやファームウェアを転送するときなどに
　使用されているデータ転送プロトコルです。  
　使い勝手が簡単で、プロトコルも簡単なので、機器のメンテナンスに向いています。
　認証や暗号化は行わないので、転送時のみ有効にする、もしくは侵入・ファイル転送
　されても問題の無い用途で利用します。

Raspberry PiへのTFTPサーバのインストール方法
    $ sudo apt-get install tftpd-hpa
    
    設定ファイル(/etc/default/tftpd-hpa) 例
    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"

TFTPサーバ上のファイル作成
    $ sudo touch /srv/tftp/tftpc_1.txt
    $ sudo chmod 666 /srv/tftp/tftpc_1.txt
    $ sudo chmod 755 /srv/tftp

TFTPサーバの起動
    $ sudo /etc/init.d/tftpd-hpa start

TFTPサーバの停止
    $ sudo /etc/init.d/tftpd-hpa stop

転送用のファイルを保存
    $ echo -e 'Hello!\nThis is from Rasberry Pi' > /srv/tftp/tftpc_1.txt

【注意事項】
・TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器が
　セキュリティの脅威にさらされた状態となります。
・また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する
　恐れが高まります。
・インターネットに接続すると外部からの侵入される場合があります。
・TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
・TFTPが不必要なときは、停止させてください。
*******************************************************************************/

#include <M5StickC.h>                           // M5StickC用ライブラリ
#include <WiFi.h>                               // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#include <SPIFFS.h>
#define SSID "iot-core-esp32"                   // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define TFTP_SERV {192,168,0,XXX}               // TFTPサーバのIPアドレス
#define PORT 1024                               // 送信のポート番号
#define DEVICE "tftpc_1,"                       // デバイス名(5字+"_"+番号+",")
#define FILENAME "/tftpc_1.txt"                 // ファイル名(表示用)
#define TIMEOUT 6                               // 電源を切るまでの時間(秒)

int BTN_LONG = 0;                               // 0:非押下 1:長押済 -1:初期押
int count = TIMEOUT;                            // 電源を切るまでの残り時間

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(M5_LED,OUTPUT);                     // LEDのIOを出力に設定
    M5.begin();                                 // M5StickC用Lcdライブラリの起動
    M5.Axp.ScreenBreath(7+1);                   // LCDの輝度を1に設定
    BTN_LONG = M5.BtnA.read();
    if( BTN_LONG ){
        WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
        WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    }else{
        WiFi.mode(WIFI_OFF);                    // 無線LANをＯＦＦに設定
    }
    if(M5.BtnB.read() || !SPIFFS.begin()){      // ファイルシステムSPIFFSの開始
        M5.Lcd.print("Formating SPIFFS...");
        SPIFFS.format(); SPIFFS.begin();        // エラー時にSPIFFSを初期化
        M5.Lcd.println("Done");
    }
}

void loop() {
    WiFiUDP udp;                                // UDP通信用のインスタンスを定義
    char data[512];                             // TFTPデータ用変数
    int len_tftp=-1;                            // TFTPデータ長

    if(BTN_LONG == -1){
        if( M5.BtnA.read() ){
            BTN_LONG = 1;
            WiFi.mode(WIFI_STA);                // 無線LANをSTAモードに設定
            WiFi.begin(SSID,PASS);              // 無線LANアクセスポイントへ接続
            count = TIMEOUT;                    // タイムアウトを初期値に
        }else{
            BTN_LONG = 0;
        }
    }else if(BTN_LONG == 0 && M5.BtnA.read() ) BTN_LONG = -1;

    int bvus_mV = M5.Axp.GetVusbinData()* 1.7f; // USB電圧を取得
    int batt_mV = M5.Axp.GetVbatData() * 1.1f;  // 電池電圧を取得
    int batt_mA = M5.Axp.GetIchargeData()/2
                - M5.Axp.GetIdischargeData()/2; // 充電放電電流を取得
    int stat = WiFi.status();
    M5.Lcd.setCursor(0,0);
    M5.Lcd.print("Ex26 TFTP LCD");              // タイトルを表示
    File file = SPIFFS.open(FILENAME,"r");      // 読み取りのためにファイルを開く
    while(file){
        if(file.available()) M5.Lcd.write(file.read());
        else file.close();
    }
    M5.Lcd.printf("WiFi %d  \n",(int8_t)((byte)stat));
    M5.Lcd.printf("UsbV %1.3f  \n",bvus_mV/1000.);
    M5.Lcd.printf("BatV %1.3f  \n",batt_mV/1000.);
    M5.Lcd.printf("BatA %1.3f  \n",batt_mA/1000.);
    M5.Lcd.printf("Time %d/%d \n",count,TIMEOUT);
    
    if(stat == WL_CONNECTED){                   // 接続に成功したとき
        tftpStart(TFTP_SERV);                   // TFTPの開始
        len_tftp = tftpGet(data);               // TFTP受信(data=受信データ)
        IPAddress IP = WiFi.localIP();          // 本機のIPアドレスを取得
        IP[3] = 255;                            // ブロードキャストアドレスに
        if(len_tftp > 0){
            M5.Lcd.fillScreen(BLACK);           // LCDを消去
            M5.Lcd.setCursor(0,0);              // 文字描画位置を画面左上へ
            M5.Lcd.println("Recieved:");        // 受信表示
            M5.Lcd.println(data);               // 受信文字列を表示
            udp.beginPacket(IP, PORT);          // UDP送信先を設定
            udp.print(DEVICE);                  // デバイス名を送信
            udp.print(len_tftp);                // 受信長を送信
            udp.printf(", %d",bvus_mV);         // 変数bvus_mVの値を送信
            udp.printf(", %d",batt_mV);         // 変数batt_mVの値を送信
            udp.printf(", %d",batt_mA);         // 変数batt_mAの値を送信
            udp.print(", ");                    // カンマを送信
            udp.println(millis()/1000 + 0.2,1); // 起動後の秒数を送信
            udp.endPacket();                    // UDP送信の終了(実際に送信する)
            delay(200);                         // 送信待ち時間
            BTN_LONG = 0;
            count = TIMEOUT;                    // タイムアウトを初期値に
            WiFi.disconnect();                  // WiFiアクセスポイントを切断
            WiFi.mode(WIFI_OFF);                // 無線LANをＯＦＦに設定
            File file = SPIFFS.open(FILENAME,"w");
            if(file){
                file.println(data);
                file.close();
            }
        }
    }
    digitalWrite(M5_LED,LOW);   delay(10);      // LEDの点灯
    digitalWrite(M5_LED,HIGH);  delay(990);     // LEDの消灯
    if( bvus_mV > 4500) return;                 // USB接続中は繰り返し実行
    if( M5.BtnA.read() ) return;                // ボタン押下状態で繰り返し実行
    if( count <= 0 ) sleep();                   // タイムアウト時にSleepへ
    count--;
}

void sleep(){
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
    delay(200);                                 // 送信待ち時間
    
    pinMode(BUTTON_A_PIN,INPUT_PULLUP);
    while(digitalRead(BUTTON_A_PIN) == LOW);
    TimerWakeUp_setExternalInput((gpio_num_t)BUTTON_A_PIN, LOW);
//  TimerWakeUp_setSleepTime((int)(SLEEP_P/1000000ul));
    TimerWakeUp_sleep();
}
