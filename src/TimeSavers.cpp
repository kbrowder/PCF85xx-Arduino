/*
 * TimeSavers.cpp
 *
 *  Created on: Mar 14, 2012
 *      Author: KBrowder
 */
#include "TimeSavers.h"
#include <EEPROM.h>
#include <Time.h>
AbstractTimeSaver::~AbstractTimeSaver() {
}

RAMTimeSaver::RAMTimeSaver() {
	this->init();
}
void RAMTimeSaver::init() {
	this->lastTime=0;
	this->timeSet=false;
}
void RAMTimeSaver::save(timems_t time) {
	this->lastTime = time;
	this->timeSet = this->lastTime!=(timems_t)-1;
}
timems_t RAMTimeSaver::load() {
	if (this->timeSet==false) {
		return (timems_t)-1;
	}
	return this->lastTime;
}


EEPROMTimeSaver::EEPROMTimeSaver() {
	this->EEPROM_ADDR = 0;
	this->init();
}
EEPROMTimeSaver::EEPROMTimeSaver(uint8_t address) {
	this->EEPROM_ADDR = address;
	this->init();
}

void EEPROMTimeSaver::save(timems_t time) {
	this->RAMTimeSaver::save(time);
	for (unsigned int i = 0; i < sizeof(time); i++) {
		EEPROM.write((int) (this->EEPROM_ADDR + i),
				(uint8_t) ((time >> (8 * i)) & 0xFF));
	}
}

timems_t EEPROMTimeSaver::load() {
	timems_t cached = this->RAMTimeSaver::load();
	if ((!cached) != 0) {
		return cached;
	}
	timems_t result = 0;
	for (unsigned int i = 0; i < sizeof(result); i++) {
		uint8_t d = EEPROM.read((int) (EEPROM_ADDR + i));
		result |= ((uint64_t) d) << (8 * i);
	}
	return result;
}
