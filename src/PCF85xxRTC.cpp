
#include "PCF85xxRTC.h"
#include "Stream.h"
#include <EEPROM.h>
struct DAY_YEAR {
	uint8_t day_u :4;
	uint8_t day_t :2;
	uint8_t year_off :2;
};
struct WEEKDAY_MONTH {
	uint8_t month_u :4;
	uint8_t month_t :1;
	uint8_t dow :3;
};
struct HOURS {
	uint8_t hour_u :4;
	uint8_t hour_t :2;
	bool am :1;
	bool is24hr :1;
};

inline uint8_t from_bcd(uint8_t in) {
	return 10 * (in >> 4) + (in & 0xF);
}
inline uint8_t to_bcd(uint8_t in) {
	return ((in / 10) << 4) | (in % 10);
}
const uint8_t PCF85xx::READ_ADDR = 0xA2 >> 1;
const uint8_t PCF85xx::WRITE_ADDR = 0xA3 >> 1;

const uint8_t PCF85xx::EEPROM_ADDR = 0;
PCF85xx PCF85xx::defaultRTC;

void PCF85xx::initControlReg() {
	this->controlReg.clear();
}
time_t PCF85xx::timeFromEEPROM() {
	/* returning !0 means unset */
	time_t result = 0;
	for (int i = 0; i < 4; i++) {
		uint8_t d = EEPROM.read((int) (EEPROM_ADDR + i));
		result |= ((uint32_t) d) << (8 * i);
	}
	return result;

}

void PCF85xx::timeToEEPROM(time_t time) {
	for (int i = 0; i < 4; i++) {
		EEPROM.write((int) (EEPROM_ADDR + i),
				(uint8_t) ((time >> (8 * i)) & 0xFF));
	}
}

PCF85xx::PCF85xx() {
	this->init(TwoWire());
}
PCF85xx::PCF85xx(TwoWire wire) {
	this->init(wire);
}
void PCF85xx::init(TwoWire wire) {
	this->wire = wire;
	this->wire.begin();
}

PCF85xx * PCF85xx::getDefaultRTC() {
	return &(PCF85xx::defaultRTC);
}
time_t PCF85xx::getDefaultTime() {
	return PCF85xx::getDefaultRTC()->get();
}
void PCF85xx::setDefaultTime(time_t t) {
	PCF85xx::getDefaultRTC()->set(t);
}
void PCF85xx::setup() {
	this->writeByte(this->STATUS_CONTROL_REG, this->controlReg.toInt());
}
void PCF85xx::reset() {
	this->writeByte(this->STATUS_CONTROL_REG, 0);

	tmElements_t tm;
	tm.Year = CalendarYrToTm(2011);
	tm.Month = 12;
	tm.Day = 31;
	tm.Wday = 6;
	tm.Hour = 23;
	tm.Minute = 59;
	tm.Second = 59;

	this->write(tm);

}

time_t PCF85xx::get() {
	tmElements_t tm;
	this->read(tm);
	return makeTime(tm);
}

void PCF85xx::read(tmElements_t &tm) {
	this->wire.beginTransmission(this->READ_ADDR);
	this->wire.write(this->SEC_REG);
	this->wire.endTransmission();
	const size_t num_results = 5;
	this->wire.requestFrom(this->READ_ADDR, num_results);
	uint8_t results[num_results];
	this->wire.readBytes((char*) results, num_results);

	tm.Second = from_bcd(results[0]);
	tm.Minute = from_bcd(results[1]);
	//for hour the two msb are The "12 Hour flag" (bit 7)
	//  and the subordinant "pm flag" (bit 6)

	HOURS * hour = (struct HOURS *) (results + 2);
	tm.Hour = hour->hour_t * 10 + hour->hour_u;

	DAY_YEAR * day_year = (struct DAY_YEAR *) (results + 3);
	tm.Day = day_year->day_t * 10 + day_year->day_u;
	uint16_t eeprom_year = year(this->timeFromEEPROM());
	uint16_t year = eeprom_year + (uint16_t) (day_year->year_off);
	tm.Year = CalendarYrToTm(year);

	WEEKDAY_MONTH * wdm = (struct WEEKDAY_MONTH *) (results + 4);
	tm.Wday = wdm->dow;
	tm.Month = wdm->month_t * 10 + wdm->month_u;

	//update eeprom if year_off > 0 so next time year_off=0
	// ensuring we're never overflow 3 bits.
	if (day_year->year_off > 0) {
		//Serial.println("Updating eeprom time on read");
		this->timeToEEPROM(makeTime(tm));
		day_year->year_off = 0;
		uint8_t * dy_ptr = results + 8;
		dy_ptr = (uint8_t*) day_year;
		this->writeByte(this->YEAR_REG, *dy_ptr);
	}
}
void PCF85xx::set(time_t t) {
	tmElements_t tm;
	breakTime(t, tm);
	this->write(tm);
}
void PCF85xx::write(tmElements_t &tm) {
	time_t eeprom_time = this->timeFromEEPROM();
	int year_off = (int) tmYearToCalendar(tm.Year) - (int) year(eeprom_time);
	if ((!eeprom_time) == 0 || year_off >= 3 || year_off < 0) { //unset eeprom or bad offsets
		//Serial.println("Updating eeprom time on write");
		eeprom_time = makeTime(tm);
		this->timeToEEPROM(eeprom_time);
		year_off = 0;
	}

	HOURS hour;
	hour.is24hr = hour.am = 0;
	hour.hour_t = tm.Hour / 10;
	hour.hour_u = tm.Hour % 10;
	uint8_t * hour_i = (uint8_t*) &hour;

	DAY_YEAR dy;
	dy.day_t = tm.Day / 10;
	dy.day_u = tm.Day % 10;

	dy.year_off = year_off;
	uint8_t * day_i = (uint8_t*) &dy;

	WEEKDAY_MONTH wm;
	wm.month_t = tm.Month / 10;
	wm.month_u = tm.Month % 10;
	wm.dow = tm.Wday;
	uint8_t * wm_i = (uint8_t*) &wm;

	this->wire.beginTransmission(this->WRITE_ADDR);
	this->wire.write(this->SEC_REG); //Starting address

	this->wire.write(to_bcd(tm.Second));

	this->wire.write(to_bcd(tm.Minute));

	this->wire.write(*hour_i);

	this->wire.write(*day_i);

	this->wire.write(*wm_i);

	this->wire.endTransmission();
}
void PCF85xx::writeByte(uint8_t word, uint8_t value) {
	this->wire.beginTransmission(this->WRITE_ADDR);
	this->wire.write(word);
	this->wire.write(value);
	this->wire.endTransmission();
}
uint8_t PCF85xx::readByte(uint8_t word) {
	char result = 0;
	this->wire.beginTransmission(this->READ_ADDR);
	this->wire.write(word);
	this->wire.endTransmission();
	this->wire.requestFrom(this->READ_ADDR, (uint8_t) 1);
	if (1 <= this->wire.available()) {
		this->wire.readBytes(&result, (long unsigned int) 1);
	}
	return (uint8_t) result;
}
uint8_t PCF85xx::readHundredths() {
	return from_bcd(this->readByte(this->HUNDRETH_SEC_REG));

}

ControlRegister::ControlRegister() {
	this->clear();
}
void ControlRegister::clear() {
	memset(&this->controlRegisterData, 0, sizeof(this->controlRegisterData)); //clear controlReg
}

uint8_t ControlRegister::toInt() {
	return *(uint8_t *) &this->controlRegisterData;
}

ControlRegister::CONTROL_REG * ControlRegister::getData() {
	return &this->controlRegisterData;
}
