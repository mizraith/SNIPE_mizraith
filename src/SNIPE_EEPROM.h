//
// Created by Red Byer on 10/16/23.
//

#ifndef SNIPE_MIZRATIH_SNIPE_EEPROM_H
#define SNIPE_MIZRATIH_SNIPE_EEPROM_H

#include <Arduino.h>
#include <EEPROM.h>

// MUST come after the SID, which starts typically @0 and goes for 30 bytes...
#define kSETTINGS_START_ADDRESS  100

struct user_settings {
    uint8_t  snipe_version;  //0  after given time in a mode, we will save it.
    uint8_t sl1_id;         //1
    uint8_t sl1_numpixels;  //2
    uint8_t sl2_id;         //3  number of times mode 1 has been selected/saved, track for fun
    uint8_t sl2_numpixels;  //4
    uint8_t sl3_id;         //5
    uint8_t sl3_numpixels;  //16
};


template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)  {
        EEPROM.write(ee++, *p++);
    }
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++) {
        *p++ = EEPROM.read(ee++);
    }
    return i;
}



# pragma mark EEPROM Utils
uint8_t isVirginEEPROM();
void initEEPROM(int, uint8_t);

void loadSettingsFromEEPROM(struct user_settings *);
void printUserSettings(struct user_settings *);


#endif //SNIPE_MIZRATIH_SNIPE_EEPROM_H
