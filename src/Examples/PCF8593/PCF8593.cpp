/**
 * \author Kevin Browder
 * \copyright GNU Public License.
 **/
#include "PCF8593.h"
#include "../../PCF85xxRTC.h"
#include "Arduino.h"
#include "Stream.h"
#include <Time.h>

void setup() {
    Serial.begin(9600);
    Serial.println("started");
    PCF85xx::getDefaultRTC()->setup();
    PCF85xx::getDefaultRTC()->reset();
    setSyncProvider( PCF85xx::getDefaultTime );
}

void loop() {
    char str[256];
    snprintf(str, 256, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(),
           day(), hour(), minute(), second());
    Serial.println(str);
    delay(5000);
}
