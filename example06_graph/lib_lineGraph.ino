#include <M5Stack.h>
#define TFT_GREY 0x5AEB

int lineGraphMinVal = 0;
int lineGraphMaxVal = 100;
byte lineGraphVal[280];
int lineGraphVal_n=0;

void lineGraphCls(){
	M5.Lcd.setTextColor(TFT_BLACK);
	M5.Lcd.fillRect(5, 4, 310, 208, TFT_WHITE);
	for (int i = 0; i <= 14; i++) {
		int x = 30 + i * 20;
		M5.Lcd.drawLine(x, 8, x, 208, TFT_LIGHTGREY);
	}
	for (int i = 0; i <= 10; i++) {
		int y = 8 + i * 20;
		M5.Lcd.drawLine(30, y, 310, y, TFT_LIGHTGREY);
		y = 200 - i * 20 + 4;
		if(i == 10) y++;
		M5.Lcd.drawRightString(String(map(i,0,10,lineGraphMinVal,lineGraphMaxVal)), 28, y, 1);
	}
}

void lineGraphInit(){
	M5.Lcd.setTextSize(1);
	M5.Lcd.fillRect(0, 0, 319, 215, TFT_GREY);
	lineGraphCls();
	M5.Lcd.setCursor(0,216);
	M5.Lcd.setTextColor(TFT_WHITE);
}

void lineGraphInit(int min_val, int max_val){
	lineGraphMinVal = min_val;
	lineGraphMaxVal = max_val;
	lineGraphInit();
}

void lineGraphPlot(float value_f){
	float delta = (float)(lineGraphMaxVal - lineGraphMinVal);
	if( delta != 0.){
		value_f = ( value_f - (float)lineGraphMinVal ) * 200. / delta;
	}
	int value;
	if(value_f > 0. ) value = (int)(value_f + 0.5);
	else              value = (int)(value_f - 0.5);

	if(value < -4) value = -4;
	if(value > 204) value = 204;
	lineGraphVal[lineGraphVal_n] = (byte)(208 - value);
	if(lineGraphVal_n > 0){
		if(lineGraphVal_n >= 279){
			lineGraphCls();
			for(int i=0;i<260;i++) lineGraphVal[i] = lineGraphVal[i+20];
			for(int i=1;i<259;i++){
				int x = 30 + i;
				int y = (int)lineGraphVal[i];
				M5.Lcd.drawLine(x, y, x-1, (int)lineGraphVal[i-1], TFT_BLACK);
			}
			lineGraphVal_n = 259;
		}
		int i = lineGraphVal_n;
		int x = 30 + i;
		int y = (int)lineGraphVal[i];
		M5.Lcd.drawLine(x, y, x-1, (int)lineGraphVal[i-1], TFT_BLACK);
	}
	lineGraphVal_n++;
	M5.Lcd.setTextColor(TFT_WHITE);
}
