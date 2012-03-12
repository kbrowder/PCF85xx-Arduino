
#ifndef PCF85xxRTC_H_
#define PCF85xxRTC_H_
#include "Arduino.h"
#include <Wire.h>
#include <Time.h>


uint8_t from_bcd(uint8_t in);
uint8_t to_bcd(uint8_t in);
class ControlRegister {
public:
	enum FUNCTION_MODES {
		CLOCK_32KHZ, CLOCK_50HZ, EVENT_COUNTER, TEST_MODE
	};
	struct CONTROL_REG {
		bool TIMER_OR_SEC_FLAG :1;
		bool ALARM_OR_MIN_FLAG :1;
		bool ALARM_ENABLE :1;
		bool MASK_FLAG :1;
		FUNCTION_MODES FUNCTION_MODE :2;
		bool HOLD_LAST_COUNT :1;
		bool COUNTING_FLAG :1;
	};
	ControlRegister();
	uint8_t toInt();
	void clear();
	CONTROL_REG * getData();
private:
	CONTROL_REG controlRegisterData;

};
class PCF85xx {
private:
	ControlRegister controlReg;

	static const uint8_t READ_ADDR;
	static const uint8_t WRITE_ADDR;
	enum REGISTERS {
		STATUS_CONTROL_REG,
		HUNDRETH_SEC_REG,
		SEC_REG,
		MIN_REG,
		HOUR_REG,
		YEAR_REG,
		MONTH_REG,
		DAY_REG,
		ALARM_CONTROL_REG
	};
	static PCF85xx defaultRTC;
protected:
	TwoWire wire;
	const static uint8_t EEPROM_ADDR;
	void initControlReg();
	time_t timeFromEEPROM();
	void timeToEEPROM(time_t);
public:

	PCF85xx();
	PCF85xx(TwoWire);
	static PCF85xx * getDefaultRTC();
	static time_t getDefaultTime();
	static void setDefaultTime(time_t t);
	void init(TwoWire);
	void setup();
	void reset();
	time_t get();
	void set(time_t t);
	void read(tmElements_t &tm);
	void write(tmElements_t &tm);
	void writeByte(uint8_t word, uint8_t value);
	uint8_t readByte(uint8_t word);
	uint8_t readHundredths();

};
time_t PCF85xx_get();
void PCF85xx_set(time_t t);




#endif /* PCF85xxRTC_H_ */
