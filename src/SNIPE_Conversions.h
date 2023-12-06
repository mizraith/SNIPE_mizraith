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
#include <stdio.h>
#include <limits.h>

# pragma mark Color <-> String Conversions
char *unsigned_to_hex_string(uint32_t x, char *dest, size_t size);
char *color_uint_to_hex_string(uint32_t x, char *dest, size_t size);
uint32_t color_uint_from_hex_string(char * s);

// Use the NO_0x size if you create a macro and don't want the extra "0x" at the beginning.
// Difference is the +1 vs. +3 for the extra characters.   CHAR_BIT is typically 8.
// 4 byte unsigned needs    (((4 * 8) + 3 ) / 4)  + 3 = 11           0xABCDEFGH/0
//
//#define UNS_HEX_STR_SIZE_NO_0x ((sizeof(unsigned) * CHAR_BIT + 3)/4 + 1)
#define UNS_HEX_STR_SIZE ((sizeof(uint32_t) * CHAR_BIT + 3)/4 + 3)
//#define UNS_HEX_STR_SIZE 9

// ---------- The following do not work on C++ as you cannot index into a temporary array like this. -----------
//#pragma mark Method Shortcuts
/**
 * Convenience DEFINES for our color-to-hex string functions.
 */
//                         compound literal v--------------------------v
//#define U2HS(x) unsigned_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)
//#define C2HS(x) color_uint_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)
// -------------------------------------------------------------------------------------------------------------

// CONVERSIONS
# pragma mark HexStr <-> Bytes
void convertHexStringToByteArray(char *, uint8_t *);
void convertByteArrayToHexString(byte *, uint8_t, char *);
byte getHexNibbleFromChar(char);
char getCharFromHexNibble(uint8_t);

# pragma mark Base32
String ltob32( unsigned long i );

# pragma mark String <-> Numbers
void get2CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get3CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get4CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get5CharStringFromNumber(char *buffstr, const char *padding, uint16_t value);
void get4CharTimeStringFromSeconds(char *buffstr, uint8_t time);
void get5CharTimeStringFromSeconds(char *buffstr, uint16_t time);



#endif