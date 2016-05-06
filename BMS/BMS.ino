//#define espdisable

#include "ESP8266.h"
#include "OZ890.h"
#include <Wire.h>

double vTot=0;
double amp,Ah=0;
long lasttime;
byte val1,val2,stat1,stat2;
bool first=true;


void setup() {
	Serial.begin(115200);
	#ifdef __AVR_ATmega2560__
		Serial1.begin(115200);
	#endif
	//ATcommand("AT+RST","ready");
	ATcommand("ATE0","OK");
	connectWiFi();
	connectUdp();
}

	
void loop() {


String s;

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
  
  s+="Cell "+String(cell+1)+":"+String(volt1/1000.0,3)+"\r\n";
}

  UDPSend(s);
  UDPSend("Pack:"+String(vTot,2));


  int adr = 0x52;         
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
  double temp = ((0.61 * b)-431.51) / 2.0976-40;
  UDPSend("Temperature:"+String(temp,2));
  
  /*
    int raw=read(0x4C)>>3;
 
    Serial.println("ExttRaw: "+String(raw)+" "+String(raw,BIN));
    double exttemp = Thermistor(raw);
    Serial.println("ExtTemp: "+String(exttemp));
	double volt=raw*1.22;
    Serial.println("ExtTemp: "+String(volt));
   
   */
   
   
  adr = 0x54;
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
  
  amp = getCurrent() / 1000.0;
  //amp = 7.63 * a /2000.0; // mV-Spannungsabfall
  //amp = (val2 * 256 + val1)/1000.0*7.63; // mV-Spannungsabfall

  if (!first) {
	  Ah+=amp*(millis()-lasttime)/1000.0/60.0/60.0;
  }
  lasttime=millis();
  
/*   Serial.print("Amp1, 2:\t\t");
   Serial.print(val1);
   Serial.print("  ");
   Serial.println(val2);*/
   UDPSend("A:"+String(-amp,3));
   UDPSend("Ah:" + String(-Ah, 3));
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
   
   delay(250);
   
   vTot=0;
  
}


double Thermistor(int RawADC) {
	double Temp;
	Temp = log(10000.0*((8192.0/RawADC-1)));
	//         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
	Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
	Temp = Temp - 273.15;            // Convert Kelvin to Celcius
	//Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
	return Temp;
}


