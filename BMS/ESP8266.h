/*
 * ESP8266.h
 *
 * Created: 27.03.2016 23.01.55
 *  Author: Amund Børsand
 */ 

#include <Arduino.h>

#ifndef ESP8266_H_
#define ESP8266_H_



#define SSID "Yeehaw2"
#define PASS "badbadbadb"

bool WiFiStatus = false;
bool UdpStatus = false;
bool rxdisconnected=false;


String censor(String s) {
	s.replace("AT","**");
	return s;
}


int ATcommand(String command,String success="", String failure = "") {
	String t="";
	//Serial.println("Sending command '"+censor(command)+"'");
	//t=censor(Serial.readString());
#ifdef __AVR_ATmega2560__
	Serial1.println(command);
#endif
	Serial.println(command);
	if (success=="") {
		delay(10);
		return 0;
	}
//#ifndef espdisable
	int fails=0;
	while (fails < 20) {		
#ifdef __AVR_ATmega2560__
		t+=Serial1.readString();
#else
		t+= censor(Serial.readStringUntil('\n'));
#endif

		if (t.indexOf(success)>=0) {
			Serial.println("'"+success+"' found!");
			return 0;
		}
		if (failure!="")
		if (t.indexOf(failure)>=0) {
			Serial.println("'"+failure+"' found! Chip said "+censor(t));
			return 1;
		}
		fails++; // we have to give up at some point, maybe we lost a feedback or we're out of sync with the chip
		//Serial.print(".");
	}
#ifdef __AVR_ATmega2560__
	Serial.println((command) + " gave up after " + fails + " tries. Chip said " + censor(t));
#else
	Serial.println(censor(command) + " gave up after " + fails + " tries. Chip said " + censor(t));
#endif
//#endif
	return 3;
}

boolean connectWiFi() {
	ATcommand("AT+CWAUTOCONN=0","OK");
	ATcommand("AT+CWMODE=1","OK");
	String cmd = "AT+CWJAP=\"";
	cmd += SSID;
	cmd += "\",\"";
	cmd += PASS;
	cmd += "\"";
	int result=ATcommand(cmd,"CONNECTED","FAIL");
	if (result==0) {
		Serial.println("Yeehaw found!");
		WiFiStatus=true;
		return true;
	}
	else {
		Serial.println("Yeehaw not found");
		WiFiStatus=false;
		ATcommand("AT+CWMODE=2","OK");		
		ATcommand("AT+CIPAP=\"192.168.0.1\"","OK");
		return false;
	}
}

boolean connectUdp() {
	ATcommand("AT+CIPCLOSE","OK","FAIL");
	ATcommand("AT+CIPMUX=0","OK","ERROR");
	String udpcommand;
	udpcommand = "AT+CIPSTART=";
	udpcommand += "\"UDP\",";
	udpcommand += "\"192.168.0.255\"";
	udpcommand += ",4444";
	ATcommand(udpcommand,"OK");
}

void UDPSend(String s) {
	String udpcommand = "AT+CIPSEND=";
	udpcommand += s.length();
	ATcommand(udpcommand);
	ATcommand(s);
}

#endif /* ESP8266_H_ */