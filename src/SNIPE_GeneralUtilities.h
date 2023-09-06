/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */
#ifndef _SNIPE_GeneralUtilities_H_
#define _SNIPE_GeneralUtilities_H_

#include <Arduino.h>
#include <Wire.h>

# pragma mark P String Utils
void printString_P(const char *);

# pragma mark RAM Utils
int freeRam();
void checkRAMandExitIfLow(uint8_t);
void gotoEndLoop();
//void checkRAM();

# pragma mark EEPROM Utils
uint8_t isVirginEEPROM();
void initEEPROM(int, uint8_t);

# pragma mark I2C / Wire Helpers
//static inline void wiresend(uint8_t);
//static inline uint8_t wirereceive();

static inline void wiresend(uint8_t x) {
#if ARDUINO >= 100
    Wire.write((uint8_t)x);
#else
    Wire.send(x);
#endif
}

static inline uint8_t wirereceive(void) {
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}



#endif