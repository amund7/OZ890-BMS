/* Original:
Copyright (c) 2013, G. Dick <demdick@googlemail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, print to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


Brutally adapted for Arduino and my own needs by Amund Børsand

*/


#include "OZ890.h"
#include <Wire.h>

void setup() {
	Serial.begin(115200);

	//putEepromByte(0x25, 0);
	//putEepromByte(0x04 + 5, 0); // 1.22);
	for (uint8_t readWordAddress = 0; readWordAddress <= 127; readWordAddress++) {
		// dump eeprom
		Serial.print(getEepromByte(readWordAddress), HEX);
		Serial.print(" ");
	}

	Serial.println("Password: ");
	Serial.print(getEepromByte(0x7E), HEX);
	Serial.print(" ");
	Serial.print(getEepromByte(0x7D), HEX);
	Serial.print(" ");
}



void loop() {
	
	//displayStatus();

	displaySettings();

	//Serial.println("MAIN MENU");
	//Serial.pr


	delay(2000);
}



void displayStatus() {
	uint8_t byte;

	// new page
	Serial.print(("\r\n"));

	Serial.print(("Voltages\r\n--------\r\n"));
	uint16_t packVoltage = 0;
	for (uint8_t i = 1; i <= 12; i++) {
		Serial.print(("Cell"));
		Serial.print(i);
		Serial.print((": "));
		if (i < 10) Serial.print(" ");

		uint16_t cellVoltage = getCellVoltage(i);
		packVoltage += cellVoltage;

		Serial.print(String(cellVoltage / 1000.0,3));
		Serial.print(" V\t");

		Serial.print(" Offset: ");
		Serial.print(getEepromByte(0x05 + i - 1)*1.22);
		Serial.print(" mV\t");
		Serial.print(getEepromByte(0x05 + i - 1));
		Serial.print(" raw");

		//if (!(i % 4)) 
		Serial.print(("\r\n"));
	}

	Serial.print(("\r\n\r\nEntire battery pack voltage: "));
	Serial.print(packVoltage/1000.0);
	Serial.print(" V");

	Serial.print(("\r\n\r\nStatus: charge / discharge\r\n--------------------------\r\n"));
	byte = read(0x20);
	Serial.print(("Charge: "));
	sendStatusBit(byte & (1 << 3));
	Serial.print(("\tDischarge: "));
	sendStatusBit(byte & (1 << 2));
	Serial.print("\r\nActual current: ");
	Serial.print(getCurrent());
	Serial.print("\r\nSense Resistor:");
	Serial.print(senseResistor/10.0);
	Serial.print(" mOhm");

	Serial.print(("\r\n\r\nStatus: failures\r\n----------------\r\n"));
	byte = read(0x1c);
	Serial.print(("UV:   "));
	sendStatusBit(byte & (1 << 0));
	Serial.print(("\tOV : "));
	sendStatusBit(byte & (1 << 5));
	Serial.print(("\t\tUT:   "));
	sendStatusBit(byte & (1 << 6));
	Serial.print(("\tOT:   "));
	sendStatusBit(byte & (1 << 7));
	Serial.print(("\r\nCUPF: "));
	sendStatusBit(byte & (1 << 4));
	Serial.print(("\tMPF: "));
	sendStatusBit(byte & (1 << 3));
	Serial.print(("\t\tVHPF: "));
	sendStatusBit(byte & (1 << 2));
	Serial.print(("\tVLPF: "));
	sendStatusBit(byte & (1 << 1));

	Serial.print(("\r\n\r\nStatus: waiting for release\r\n---------------------------\r\n"));
	byte = read(0x1f);
	Serial.print(("UV: "));
	sendStatusBit(byte & (1 << 1));
	Serial.print(("\t\tOV: "));
	sendStatusBit(byte & (1 << 0));
	Serial.print(("\t\tOC: "));
	sendStatusBit(byte & (1 << 2));
	Serial.print(("\t\tSC: "));
	sendStatusBit(byte & (1 << 3));
	Serial.print(("\r\nUT: "));
	sendStatusBit(byte & (1 << 5));
	Serial.print(("\t\tOT: "));
	sendStatusBit(byte & (1 << 4));

}

void displaySettings() {
	uint8_t byte;
	uint16_t byte16;

	byte = getEepromByte(0x18);

	Serial.print(("\2\r\n(a) Permanent failures happened\r\n-------------------------------\r\n"));
	Serial.print(("Cell voltage unbalance: "));
	sendStatusBit(byte & (1 << 4));
	Serial.print(("\r\nMOSFET failure: "));
	sendStatusBit(byte & (1 << 3));
	Serial.print(("\r\nCell voltage extremely high: "));
	sendStatusBit(byte & (1 << 2));
	Serial.print(("\r\nCell voltage extremely low: "));
	sendStatusBit(byte & (1 << 1));

	byte = getEepromByte(0x26);
	Serial.print(("\r\n\r\n(b) Cell number: "));
	Serial.print(byte & 0x0f);
	Serial.print(("\r\n(c) Battery type: "));
	byte &= 0x30;
	if (byte == 0) Serial.print("30S NiMH");
	else if (byte == 16) Serial.print("20S NiMH");
	else if (byte == 32) Serial.print("Phosphate Li-ion");
	else if (byte == 48) Serial.print("Cobalt/Manganese Li-ion");

	byte = getEepromByte(0x33);
	Serial.print(("\r\n\n(d) Bleeding enabled: "));
	sendStatusBit(byte & (1 << 3));
	Serial.print(("\r\n(e) Maximum bleeding cell number: "));
	Serial.print((byte & 0x03) + 1);

	byte = getEepromByte(0x32);
	Serial.print(("\r\n(f) Bleeding all cells enabled: "));
	sendStatusBit(byte & (1 << 1));

	byte = getEepromByte(0x48);
	Serial.print(("\r\n(g) Bleeding accuracy: "));
	byte16 = (((byte & 0x07) + 1) * 8);
	byte16 *= 122;
	if (byte16 < 1000) Serial.print(byte16);
	else Serial.print(byte16);
	Serial.print(("mV"));

	Serial.print(("\r\n(h) Bleeding start voltage: "));
	sendEepromVoltageToUart(0x48);

	Serial.print(("\r\n\n(i) OV Threshold: "));
	sendEepromVoltageToUart(0x4a);

	Serial.print(("\t(j) OV Release: "));
	sendEepromVoltageToUart(0x4c);

	Serial.print(("\r\n(k) UV Threshold: "));
	sendEepromVoltageToUart(0x4e);

	Serial.print(("\t(l) UV Release: "));
	sendEepromVoltageToUart(0x50);

	Serial.print(("\r\n(m) Extremely High Voltage Threshold: "));
	sendEepromVoltageToUart(0x52);

	Serial.print(("\r\n(n) Extremely Low Voltage Threshold: "));
	sendEepromVoltageToUart(0x54);

	Serial.print(("\r\n\n(o) Gas Gauge V1: "));
	sendEepromVoltageToUart(0x66);

	Serial.print(("\r\n(p) Gas Gauge V2: "));
	sendEepromVoltageToUart(0x68);

	Serial.print(("\r\n(q) Gas Gauge V3: "));
	sendEepromVoltageToUart(0x6a);

	Serial.print(("\r\n(r) Gas Gauge V4: "));
	sendEepromVoltageToUart(0x6c);

	Serial.print(("\r\n(s) Gas Gauge V5: "));
	sendEepromVoltageToUart(0x6e);

	Serial.print(("\r\n\n(t) Factory Name: "));
	for (uint8_t i = 0x36; i <= 0x3f; i++) {
		Serial.print((char)getEepromByte(i));
	}

	Serial.print(("\r\n(u) Project Name: "));
	for (uint8_t i = 0x40; i <= 0x44; i++) {
		Serial.print((char)getEepromByte(i));
	}

	Serial.print(("\r\n(v) Version Number: "));
	Serial.print(getEepromByte(0x45));

	byte = getEepromByte(0x25);
	Serial.print(("\nATE_FRZ:   "));
	sendStatusBit(byte & (1 << 7));

	byte = getEepromByte(0x7F);
	Serial.print(("\nSTFRZ (secret data access):   "));
	sendStatusBit(byte & (1 << 7));

	byte = getEepromByte(0x33);
	Serial.print(("\nUC01 (76):   "));
	sendStatusBit(byte & (1 << 7));
	sendStatusBit(byte & (1 << 6));

	byte = getEepromByte(0x6F);
	Serial.print(("\nPWD_OK:   "));
	sendStatusBit(byte & (1 << 7));
	Serial.print(("\nPWD_FAIL:   "));
	sendStatusBit(byte & (1 << 6));
	Serial.print(("\nPWD_BUSY:   "));
	sendStatusBit(byte & (1 << 5));

}

void sendEepromVoltageToUart(uint8_t startAddress) {
	uint16_t byte16 = getVoltage(getEepromByte(startAddress), getEepromByte(startAddress + 1));
	Serial.print(String(byte16/1000.0,3));
	Serial.print(" mV");
}
