/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */
#ifndef _SNIPE_Conversions_H_
#define _SNIPE_Conversions_H_
#include <stdint.h>
#include <Arduino.h>

// CONVERSIONS
void get2CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get3CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get4CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get5CharStringFromNumber(char *buffstr, const char *padding, uint16_t value);
void get4CharTimeStringFromSeconds(char *buffstr, uint8_t time);
void get5CharTimeStringFromSeconds(char *buffstr, uint16_t time);

uint8_t addmod8( uint8_t a, uint8_t b, uint8_t m);

// High speed saturating add and subtract routines (from FastLED library)
uint8_t qadd8( uint8_t i, uint8_t j);
uint8_t qsub8( uint8_t i, uint8_t j);
uint8_t qavg8( uint8_t i, uint8_t j);

int8_t qabs8( int8_t i);
int8_t getAbs(int8_t n);

#endif