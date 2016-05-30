//#define espdisable
//#define debug // disables LCD and enables echo to serial

#include <UTFT.h>
#include <UTouch.h>
#include "ESP8266.h"

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
//extern uint8_t SevenSegNumFont[];

// Uncomment the next line for Arduino 2009/Uno
//UTFT myGLCD(ITDB32S,19,18,17,16);   // Remember to change the model parameter to suit your display module!

// Uncomment the next line for Arduino Mega
#ifndef debug
UTFT lcd(ITDB28, A5, A4, A3, A2);   // Remember to change the model parameter to suit your display module
									//UTouch touch(55,60,54,8,9);
#endif

//#include "ESP8266.h"
#include "OZ890.h"
#include <Wire.h>

double vTot=0;
double amp,amp2,Ah=0,Ah2,minAmp,maxAmp,regen,AhPos;
long lasttime;
byte val1,val2,stat1,stat2;
bool first=true;
int y = 100;

const double cellMin = 2.7;
const double cellMax = 4.2;

void setup() {
#ifdef debug
	Serial.begin(115200);
#endif
//#ifdef __AVR_ATmega2560__ // Since we are in the LCD project, we assume we are on MEGA, and having both LCD + ES8266
		Serial1.begin(115200);
//#endif
#ifndef debug
	lcd.InitLCD(PORTRAIT);
	lcd.clrScr();
	lcd.setColor(255, 255, 255);

	lcd.setBackColor(0, 0, 0);
	lcd.setFont(BigFont);
#endif
	connectWiFi();
	connectUdp();
}


void display(double val, double min, double max) {
	int d = (240 - 80)* (val - min) / (max-min);
	String s = String(val).substring(0,4);

#ifdef debug
	Serial.println(s);
#else
	lcd.setColor(255, 255, 127);
	lcd.drawRect(80, y+5, 80 + d, y + 10);
	//for (int i = 0; i < 4; i++)
	//	lcd.drawHLine(80, y+i, d);
	lcd.setColor(0);
	lcd.drawRect(81+ d, y+5, 239, y + 10);
	//for (int i = 0; i < 4; i++)
		//lcd.drawHLine(80+d, y + i, 240-d-1);
	lcd.setColor(255, 255, 255);
	lcd.print(s, 10, y);
	y += 15;
#endif
}


void loop() {


String s;
y = 10;

for (int i = 0; i < 12; i++) {
	//while (getRegister(0x25) & 1);
	uint16_t cellVoltage = getCellVoltage(i+1);
	display(cellVoltage / 1000.0, cellMin, cellMax);
	//display((cellVoltage + ((int8_t)getEepromByte(0x05 + i - 1)*1.22)) / 1000.0, cellMin, cellMax);
	vTot += cellVoltage / 1000.0;
	s += "Cell " + String(i + 1) + ":" + String((cellVoltage) / 1000.0, 3) + "\r\n";
}

  display(vTot, cellMin*12, cellMax*12);
  UDPSend(s);
  UDPSend("Pack:" + String(vTot, 2));

int adr = 0x54;
Wire.beginTransmission(i2cadr);
Wire.write(adr);
stat1 = Wire.endTransmission(false);
stat2 = Wire.requestFrom(i2cadr, 2);
val1 = Wire.read();
val2 = Wire.read();
int a = (val2 << 8) + val1;

//while (getRegister(0x25) & 1); /// wait while ADC busy

  amp = -getCurrent2() / 1000.0;
  if (first) {
	  minAmp = amp;
	  maxAmp = amp;
  }
  if (amp > maxAmp)
	  maxAmp = amp;
  if (amp < minAmp)
	  minAmp = amp;
  //amp = 7.63 * a /2000.0; // mV-Spannungsabfall
  //amp = (val2 * 256 + val1)/1000.0*7.63; // mV-Spannungsabfall

  if (!first) {
	  //accA += amp;
	  //Ah = amp/(millis() / 1000.0 / 60.0 / 60.0);
	  double instAmp= amp*(millis() - lasttime) / 1000.0 / 60.0 / 60.0;
	  Ah += instAmp;
	  if (amp > 0)
		  AhPos += instAmp;
	  if (amp < 0)
		  regen += instAmp;
  }
  //lasttime=millis();
  
  display(amp, minAmp, maxAmp);
  display(Ah, 0, 10);
  //display(AhPos, 0, 10);
  //display(-regen, 0, 10);

  UDPSend("A:"+String(amp));
  UDPSend("Ah:"+String(Ah));
  //UDPSend("Ah+:"+String(AhPos));
  //UDPSend("Regen:"+String(-regen));


 
  /*amp2 = getCurrent() / 1000.0;

  if (!first) {
	  Ah2 += amp2*(millis() - lasttime) / 1000.0 / 60.0 / 60.0;
  }
  lasttime = millis();

  //display(-amp2, minAmp, maxAmp);
  //display(-Ah2, 0, 10);
  UDPSend("A2:" + String(-amp2, 3));
  UDPSend("Ah2:" + String(-Ah2, 3));
  //UDPSend("SenseResistor:" + String(senseResistor));
  //UDPSend("A2:" + String(-getCurrent()/1000.0, 3));

  
   */
  
  if (first) {
	first=false;
  }
   
   //delay(250);
   
   vTot=0;
  
}

