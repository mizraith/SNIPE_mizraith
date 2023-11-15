//
// Created by Red Byer on 10/16/23.
//

#include "SNIPE_EEPROM.h"
#include <Arduino.h>


#pragma mark EEPROM helpers
/***************************************************
 ***************************************************
 *   EEPROM HELPERS
 ***************************************************
 ***************************************************/
// Virgin AVR EEPROM start at 0xFF, which can
// make a mess of string checking.
// This method is intended to be called during setup
// to see if an EEPROM blank is required.  This
// method basically loops through the first 10
// addresses in EEPROM and if they all equal 0xFF
// it will return 1 (true).  If not virgin, returns 0 (false).
uint8_t isVirginEEPROM() {
    uint8_t val = 0xFF;
    DEBUG_PRINTLN(F("# __Setup: Checking EEPROM:"));
    for( int i=0; i < 10; i++) {
        val = EEPROM.read(i);
        if (val != 0xFF) {
            DEBUG_PRINTLN(F(" NOT a virgin EEPROM"));
            return 0;
        }
    }
    DEBUG_PRINTLN(F(" IS a virgin EEPROM"));
    return 1;
}

// I prefer my EEPROMS init to 0
// pass in numbytes (for atmega328p should be 1024)
void initEEPROM(int numbytes, uint8_t initvalue) {
    for (int i=0; i < numbytes; i++) {
        EEPROM.write(i, initvalue);
    }
}


void loadSettingsFromEEPROM(user_settings * SETTINGS) {
    boolean settings_changed = false;
    uint16_t settings_addr = kSETTINGS_START_ADDRESS;

    DEBUG_PRINT(F("# __EEPROM Settings @: "));DEBUG_PRINT(settings_addr, DEC);DEBUG_PRINTLN();

    EEPROM_readAnything(settings_addr, *SETTINGS);

    // verify SNIPE version:
    if ((SETTINGS->snipe_version < 4) || (SETTINGS->snipe_version == 255) ) {
        SETTINGS->snipe_version = 4;
        settings_changed = true;
    }

    // verify SL1, SL2, SL3
    if (SETTINGS->sl1_id != 1) {
        SETTINGS->sl1_id = 1;           // required
        SETTINGS->sl1_numpixels = 11;   // default
        settings_changed = true;
    }

    if (SETTINGS->sl2_id != 2) {
        SETTINGS->sl2_id = 2;           // required
        SETTINGS->sl2_numpixels = 12;   // default
        settings_changed = true;
    }

    if (SETTINGS->sl3_id != 3) {
        SETTINGS->sl3_id = 3;           // required
        SETTINGS->sl3_numpixels = 13;   // default
        settings_changed = true;
    }

    if( settings_changed ) {
        DEBUG_PRINTLN(F("# __Saving Changed Settings"));
        EEPROM_writeAnything(settings_addr, *SETTINGS);
    }

    printUserSettings(SETTINGS);

}


void printUserSettings(struct user_settings * SETTINGS) {
    Serial.println(F("# __STORED SETTINGS__"));
    Serial.print(F("#     Snipe_Version : "));Serial.println(SETTINGS->snipe_version, DEC);
    Serial.print(F("#     SL1 # pixels  : ")), Serial.println(SETTINGS->sl1_numpixels, DEC);
    Serial.print(F("#     SL2 # pixels  : ")), Serial.println(SETTINGS->sl2_numpixels, DEC);
    Serial.print(F("#     SL3 # pixels  : ")), Serial.println(SETTINGS->sl3_numpixels, DEC);
    Serial.println(F("# _____________________"));
}


