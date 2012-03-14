/**
 * \author Kevin Browder
 * \copyright GNU Public License.
 **/
#ifndef PCF85xxRTC_H_
#define PCF85xxRTC_H_
#include "Arduino.h"
#include <Wire.h>
#include <Time.h>

struct tmElementsWithMillis{
	  uint8_t Second;
	  uint8_t Minute;
	  uint8_t Hour;
	  uint8_t Wday;   // day of week, sunday is day 1
	  uint8_t Day;
	  uint8_t Month;
	  uint8_t Year;   // offset from 1970;
	  uint16_t Milliseconds;
	};
class PCF85xxTypes {
protected:
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
	enum FUNCTION_MODES {
		CLOCK_32KHZ, CLOCK_50HZ, EVENT_COUNTER, TEST_MODE
	};
	struct CONTROL {
		bool TIMER_OR_SEC_FLAG :1;
		bool ALARM_OR_MIN_FLAG :1;
		bool ALARM_ENABLE :1;
		bool MASK_FLAG :1;
		FUNCTION_MODES FUNCTION_MODE :2;
		bool HOLD_LAST_COUNT :1;
		bool COUNTING_FLAG :1;
	};
	struct BCD {
		uint8_t unit :4;
		uint8_t ten :4;
	};
	struct YEAR_DAY {
		uint8_t day_u :4;
		uint8_t day_t :2;
		uint8_t year_off :2;
	};
	struct WEEKDAY_MONTH {
		uint8_t month_u :4;
		uint8_t month_t :1;
		uint8_t dow :3;
	};
	struct HOUR {
		uint8_t hour_u :4;
		uint8_t hour_t :2;
		bool am :1;
		bool is24hr :1;
	};

	enum TIMER_FUNCTION {
		NO_TIMER, HUNDRETHS, SECONDS, MINUTES, HOURS, DAYS, NOP, TESTMODE
	};
	enum ALARM_FUNCTION {
		NONE, DAILY, WEEKDAY, DATED
	};
	struct ALARM_CONTROL {
		TIMER_FUNCTION timer_func :3;
		bool timer_interrupt :1;
		ALARM_FUNCTION clock_func :2;
		bool timer_alarm :1;
		bool alarm_interrupt :1;
	};

	struct TIME {
		struct BCD hundredths;
		struct BCD seconds;
		struct BCD minutes;
		struct HOUR hours;
		struct YEAR_DAY day_year;
		struct WEEKDAY_MONTH wday_month;
	};
	struct PCF85xxREGS {
		struct CONTROL control;
		struct TIME time;
		uint8_t timer :8;
		struct ALARM_CONTROL alarm;
	};

};

inline uint64_t makeTimeMilli(tmElementsWithMillis);
inline uint64_t makeTimeMilli(tmElements_t);


class PCF85xx: PCF85xxTypes {
private:
	CONTROL controlReg;
	static PCF85xx defaultRTC;
	static const uint8_t READ_ADDR;
	static const uint8_t WRITE_ADDR;

protected:
	TwoWire wire;
	const static uint8_t EEPROM_ADDR;
	void initControlReg();
	uint64_t timeFromEEPROM();
	void timeToEEPROM(uint64_t);
	static uint8_t to_uint8(void * s);
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
	void read(tmElementsWithMillis &tm);
	void read(tmElements_t &tm);
	void write(tmElements_t &tm);
	void write(tmElementsWithMillis &tm);
	void writeByte(uint8_t word, uint8_t value);
	uint8_t readByte(uint8_t word);
	uint8_t readHundredths();

};
time_t PCF85xx_get();
void PCF85xx_set(time_t t);

#endif /* PCF85xxRTC_H_ */
