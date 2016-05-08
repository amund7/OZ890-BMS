//#define espdisable
//#define debug

#include <UTFT.h>
#include <UTouch.h>

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
double amp,Ah=0,minAmp,maxAmp,regen,AhPos;
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
	/*#ifdef __AVR_ATmega2560__
		Serial1.begin(115200);
	#endif
	*/
#ifndef debug
	lcd.InitLCD(PORTRAIT);
	lcd.clrScr();
	lcd.setColor(255, 255, 255);

	lcd.setBackColor(0, 0, 0);
	lcd.setFont(BigFont);
#endif
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

//lcd.print("test", 0, y);
//lcd.clrScr();

for (int cell=0; cell<12; cell++) {
  int adr = 0x0;         
  adr = 0x32;        
  adr = adr + 2*cell;
  Wire.beginTransmission(i2cadr); 
  Wire.write(adr); 
  stat1 = Wire.endTransmission(false); 
  stat2 = Wire.requestFrom(i2cadr, 2);
  val1 = Wire.read();
  val2 = Wire.read();
  val1 =  val1/13; 
  val2 =  (val2 & 0x7F); 
 
  double volt1 = 1.22 * (val2 * 32 + val1);

  vTot+=volt1/1000.0;
  
  display(volt1 / 1000.0, cellMin, cellMax);

}

//  UDPSend(s);
//  UDPSend("Pack:"+String(vTot,2));

  display(vTot, cellMin*12, cellMax*12);


  /*int adr = 0x52;         
  Wire.beginTransmission(i2cadr); 
  Wire.write(adr); 
  stat1 = Wire.endTransmission(false); // 0:sucess
  // register daten lesen
  stat2 = Wire.requestFrom(i2cadr, 2); // anzahl der gelesenen bytes
  val1 = Wire.read()>>3;
  val2 = Wire.read();
  // bytes auswerten
  //val1 =  val1/13; // 3 bits ausblenden 13 statt 8 eingegeben
  //val2 =  (val2 & 0x7F); // VZ ausblenden
   int b=(val2<<5) + val1;

   /*Serial.println("Val1: "+String(val1)+" "+String(val1,BIN));
   Serial.println("Val2: "+String(val2)+" "+String(val2,BIN));
   Serial.println("Word: "+String(b)+" "+String(b,BIN));
   Serial.println("Volt: "+String(b*0.61));*/
   
   //amp = 7.63 * b /2000.0; // mV-Spannungsabfall

  //double temp = (1.22 * ((val2 << 5) + val1))*2.0976/100.0;
  //double temp = ((0.61 * b)-431.51) / 2.0976-40;
//  UDPSend("Temperature:"+String(temp,2));
  
  /*
    int raw=read(0x4C)>>3;
 
    Serial.println("ExttRaw: "+String(raw)+" "+String(raw,BIN));
    double exttemp = Thermistor(raw);
    Serial.println("ExtTemp: "+String(exttemp));
	double volt=raw*1.22;
    Serial.println("ExtTemp: "+String(volt));
   
   */
   
   
  int adr = 0x54;
  Wire.beginTransmission(i2cadr); 
  Wire.write(adr); 
  stat1 = Wire.endTransmission(false);
  stat2 = Wire.requestFrom(i2cadr, 2);
  val1 = Wire.read();
  val2 = Wire.read();
  int a=(val2<<8) + val1;
  
  /*#ifdef debug
	Serial.println("Val1: "+String(val1)+" "+String(val1,BIN));
	Serial.println("Val2: "+String(val2)+" "+String(val2,BIN));
    Serial.println("Word: "+String(a)+" "+String(a,BIN));
  #endif*/
  
  amp = -getCurrent() / 1000.0;
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
  lasttime=millis();
  
  display(amp, minAmp, maxAmp);
  display(Ah, 0, 10);
  display(AhPos,0,10);
  display(-regen,0,10);

/*   Serial.print("Amp1, 2:\t\t");
   Serial.print(val1);
   Serial.print("  ");
   Serial.println(val2);*/
//   UDPSend("A:"+String(-amp,3));
//   UDPSend("Ah:" + String(-Ah, 3));
   //UDPSend("A2:" + String(-getCurrent()/1000.0, 3));

  /*adr = 0x12;
  Wire.beginTransmission(i2cadr);
  Wire.write(adr);
  stat1 = Wire.endTransmission(false);
  stat2 = Wire.requestFrom(i2cadr, 1);
  val1 = Wire.read();
  Serial.print(adr,HEX);
  Serial.print(":\t");
  Serial.println(val1,BIN);
   
  adr = 0x13;
  Wire.beginTransmission(i2cadr);
  Wire.write(adr);
  stat1 = Wire.endTransmission(false);
  stat2 = Wire.requestFrom(i2cadr, 1);
  val1 = Wire.read();
  Serial.print(adr,HEX);
  Serial.print(":\t");
  Serial.println(val1,BIN);
  */
  
   
  
  if (first) {
/*
  //adr = 0x5C;
  adr = 0x0d;
  // register adresse übermitteln
  Wire.beginTransmission(i2cadr);
  Wire.write(adr);
  stat1 = Wire.endTransmission(false); // 0:sucess
  // register daten lesen
  stat2 = Wire.requestFrom(i2cadr, 1); // anzahl der gelesenen bytes
  val1 = Wire.read();
  Serial.println(val1,BIN);
  
  val1=val1 | 0b01000000; // 7: NO ERROR DISPLAY; 6: IDLE_BLEED

	Serial.println(val1,BIN);
	delay(1000);
	//adr = 0x5C;
	
	Wire.beginTransmission(i2cadr);
	Wire.write(adr);
	//stat1 = Wire.endTransmission(true); // 0:sucess
	
	//Serial.println(stat1);

	//Wire.beginTransmission(i2cadr);
	Wire.write(val1);
	stat1=Wire.endTransmission(true);
  
	Serial.println(stat1);
*/

  /*//adr = 0x5C;
  adr = 0x12;
  // register adresse übermitteln
  Wire.beginTransmission(i2cadr);
  Wire.write(adr);
  stat1 = Wire.endTransmission(false); // 0:sucess
  // register daten lesen
  stat2 = Wire.requestFrom(i2cadr, 1); // anzahl der gelesenen bytes
  val1 = Wire.read();
  Serial.println(val1,BIN);
  
  val1=val1 | 0b00000010; // 1: BLEED ALL
  
  Serial.println(val1,BIN);
  delay(1000);
  //adr = 0x5C;
  
  Wire.beginTransmission(i2cadr);
  Wire.write(adr);
  //stat1 = Wire.endTransmission(true); // 0:sucess
  
  //Serial.println(stat1);

  //Wire.beginTransmission(i2cadr);
  Wire.write(val1);
  stat1=Wire.endTransmission(true);
  Serial.println(stat1);*/

	first=false;
  }
   
   //delay(250);
   
   vTot=0;
  
}

