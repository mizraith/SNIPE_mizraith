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
# pragma mark HexStr <-> Bytes
void convertHexStringToByteArray(char *, uint8_t *);
void convertByteArrayToHexString(byte *, uint8_t, char *);
byte getHexNibbleFromChar(char);
char getCharFromHexNibble(uint8_t);

# pragma mark Color <-> String Conversions
char *unsigned_to_hex_string(unsigned x, char *dest, size_t size);
char *color_uint_to_hex_string(unsigned x, char *dest, size_t size);
uint32_t color_uint_from_hex_string(char * s);

# pragma mark Method Shortcuts
/**
 * Convenience DEFINES for our color-to-hex string functions.
 */
// Use the NO_0x size if you create a macro and don't want the extra "0x" at the beginning.
// Difference is the +1 vs. +3 for the extra characters
#define UNS_HEX_STR_SIZE_NO_0x ((sizeof (unsigned)*CHAR_BIT + 3)/4 + 1)
#define UNS_HEX_STR_SIZE ((sizeof (unsigned)*CHAR_BIT + 3)/4 + 3)
//                         compound literal v--------------------------v
#define U2HS(x) unsigned_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)
#define C2HS(x) color_uint_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)

# pragma mark String <-> Numbers
void get2CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get3CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get4CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value);
void get5CharStringFromNumber(char *buffstr, const char *padding, uint16_t value);
void get4CharTimeStringFromSeconds(char *buffstr, uint8_t time);
void get5CharTimeStringFromSeconds(char *buffstr, uint16_t time);



#endif