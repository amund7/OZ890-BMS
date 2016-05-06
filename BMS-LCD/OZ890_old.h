#pragma once

#include <Arduino.h>
#include <Wire\Wire.h>


int i2cadr = 0x30; 


int read(int reg) {
	byte val1, val2, stat1, stat2;
	Wire.beginTransmission(i2cadr);
	Wire.write(reg);
	stat1 = Wire.endTransmission(false);
	stat2 = Wire.requestFrom(i2cadr, 2);
	val1 = Wire.read();
	val2 = Wire.read();
#ifdef debug
	Serial.println("Val1: " + String(val1) + " " + String(val1, BIN));
	Serial.println("Val2: " + String(val2) + " " + String(val2, BIN));
	Serial.println("Word: " + String(((val2 << 8) + val1)) + " " + String(((val2 << 8) + val1), BIN));
#endif
	return ((val2 << 8) + val1);
}

uint8_t eepromBusy(void) {
	uint8_t byte = read(0x5f); // EEPROM Control Register
	return (byte & (1 << 7)); // bit 7 = busy flag
}

/*void setRegister(uint8_t regAddress, uint8_t regValue) {
	this->start(OZADDR_W); // send device address
	this->write(regAddress); // address to write to
	this->write(regValue); // write value
	this->stop();
}

uint8_t getEepromByte(uint8_t address) {
	while (eepromBusy()); // wait for eeprom not be busy
	this->setRegister(0x5e, address); // set eeprom address to read

	while (this->eepromBusy());
	this->setRegister(0x5f, 0x55); // b01010101 (or 0x55) set eeprom access & word reading mode

	uint8_t byte1, byte2;
	while (this->eepromBusy());
	byte1 = this->getRegister(0x5d); // odd addr
	while (this->eepromBusy());
	byte2 = this->getRegister(0x5c); // even addr

	while (this->eepromBusy());
	this->setRegister(0x5f, 0x00); // disable eeprom access

	if (address % 2) return byte1;
	else return byte2;
}
*/


uint16_t getVoltage(uint8_t byteL, uint8_t byteH) {
	uint32_t val = (byteL >> 3) & 0b00011111;
	val = (byteH << 5) | val;
	return (val * 122) / 100;
}

uint16_t getCellVoltage(uint8_t cell) {
	if (!cell || cell > 13) return 0;

	uint8_t regAddr = 0x30 + (cell * 2);
	return getVoltage(read(regAddr), read(regAddr + 1));
}


void sendStatusBit(uint8_t bit) {
	if (bit) Serial.write(("yes"));
	else Serial.write(("no"));
}
