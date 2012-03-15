/*
 * TimeSavers.h
 *
 *  Created on: Mar 14, 2012
 *      Author: KBrowder
 */

#ifndef CHECKPOINT_H_
#define CHECKPOINT_H_
#include "Arduino.h"

typedef uint64_t timems_t;
class AbstractTimeSaver {
public:
	virtual ~AbstractTimeSaver();
	virtual void save(timems_t time)=0;
	virtual timems_t load()=0;
};

class RAMTimeSaver: public AbstractTimeSaver {
protected:
	timems_t lastTime;
	bool timeSet;
public:
	RAMTimeSaver();
	virtual void init();
	virtual void save(timems_t time);
	virtual timems_t load();
};

class EEPROMTimeSaver: public RAMTimeSaver {
protected:
	uint8_t EEPROM_ADDR;
public:
	EEPROMTimeSaver();
	EEPROMTimeSaver(uint8_t address);
	virtual void save(timems_t time);
	virtual timems_t load();
};

#endif /* CHECKPOINT_H_ */
