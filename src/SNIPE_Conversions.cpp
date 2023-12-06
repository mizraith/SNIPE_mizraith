/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */

#include "SNIPE_Conversions.h"

#include <Arduino.h>    // we make good use of String() class
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "SNIPE_Strings.h"

#pragma  mark Conversion Helpers

// Given a hex string "0A0F" convert to
// byte array  [10, 15]. ASSUMES src is sanitized [0-9A-F], and '0x' has been removed!
// and even number of characters, and target is large
// enough to handle result.   Target will be 1/2 the length of src.
void convertHexStringToByteArray( char* src, uint8_t* target ) {
    while(*src && src[1]) {
        *(target++) = getHexNibbleFromChar(*src)*16 + getHexNibbleFromChar(src[1]);
        src += 2;
    }
}

// Given an array of bytes, split each byte into 2 hex characters as a string.
// We need to know num_bytes in our array, since we cannot count on
// a null terminated string.
//     src                 target        Note that target must be  1 + 2x(src-length) to account for NULL
//     [15],[16],[26] ==>  "0F101A"      Note does not prepend "0x"
void convertByteArrayToHexString( byte* src, uint8_t numbytes, char* target ) {
    uint8_t upper, lower;
    char cupper, clower;
    uint8_t i = 0;
    uint8_t t = 0;   // start after '0x', could base it off 'i' variable, but this is more readable.
    for ( i=0; i < numbytes; i++) {
        upper = src[i] >> 4;
        lower = src[i] & 0x0F;
        cupper = getCharFromHexNibble(upper);
        clower = getCharFromHexNibble(lower);
        target[t] = cupper;
        t++;
        target[t] = clower;
        t++;
    }
    target[t] = '\0';
}

// get the 4 bit nibble 0x0000xxxx from a character
// representing a hexadecimal number
byte getHexNibbleFromChar(char ch) {
    if ((ch >= '0') && (ch <= '9')) {
        return (byte)(ch - '0');
    }
    if ((ch >= 'A') && (ch <= 'F')){
        return (byte)(ch - 'A' + 10);
    }
    if ((ch >= 'a') && (ch <= 'f')) {
        return (byte)(ch - 'a' + 10);
    }
    return (byte) 0;
}

// get the character from the 4-bit 0x0000xxxx
// nibble representing a hexadecimal number
char getCharFromHexNibble(uint8_t b) {
    if ((b >= 0) && (b <= 9)) {
        return (char)(b + '0');
    } else {
        return (char)(b - 10 + 'A');
    }
}


/**
 * unsigned_to_hex_string
 * Convert values to hex strings.
 *
 * Example use:  See /sandbox/test_hex_conversion.cpp
 *   U2HS(15);      -->  "0xF"
 *   C2HS(0x00FF00); -->  "0x00FF00"
 */
/**
 * Convenience DEFINES for our color-to-hex string functions.
 *   For necessity these are moved to the top of the file.
 */
//// Use the NO_0x size if you create a macro and don't want the extra "0x" at the beginning.
//// Difference is the +1 vs. +3 for the extra characters
//#define UNS_HEX_STR_SIZE_NO_0x ((sizeof (unsigned)*CHAR_BIT + 3)/4 + 1)
//#define UNS_HEX_STR_SIZE ((sizeof (unsigned)*CHAR_BIT + 3)/4 + 3)
////                         compound literal v--------------------------v
//#define U2HS(x) unsigned_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)
//#define C2HS(x) color_uint_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)

char *unsigned_to_hex_string(unsigned x, char *dest, size_t size) {
    //("0x%X\n", a);
    snprintf(dest, size, "%X", x);
    return dest;
}

/**
 * color_uint_to_hex_string
 * Convert color values to hex strings with fixes 6 digit length prepended with "0x"
 *
 * Example use:  See /sandbox/test_hex_conversion.cpp
 *   U2HS(15);      -->  "0x00000F"
 *   C2HS(0x00FF00); -->  "0x00FF00"
 *
 *   char *tempstr = new char[UNS_HEX_STR_SIZE];
 *   color_uint_to_hex_string(clr, tempstr, UNS_HEX_STR_SIZE);
 */
// In this cae we return at least 6 digits  0x00FF00.   Could be more if
// the color value is greater than 0xFFFFFF.
//  "0x" is prepended for us.
char *color_uint_to_hex_string(uint32_t x, char *dest, size_t size) {
    snprintf(dest, size, "0x%06lX", x);   // << need the long "l" for "lX" or it gets truncated to 16 bits!!!
    return dest;
}


/**
 * function:  color_uint_from_hex_string
 * Given a hex string ("0x1F" or "1F" convert to int value.
 * NOTE: If you give it something weird, it returns 0.
 * @param s   your c-string.   "0x0f"  "0f"  "0F"  all accepted
 * @return
 */
uint32_t color_uint_from_hex_string(char * s){
    unsigned int temp;
    uint32_t i;
    sscanf(s, "%x", &temp);
    i = 0xFFFFFF & temp;   // slam down ot 24bits
    return i;
}


char *C2HS(uint32_t x) {
    char* dest = new char[UNS_HEX_STR_SIZE];

    return color_uint_to_hex_string(x, dest, UNS_HEX_STR_SIZE);
}

/**
 * Custom function to convert a long to a bin32 representation.  Uses modulo 32 operations
 * repeatedly and indexes into the standard base32 alphabet.
 * e.g.     906,104,873 --> JBEEA3
 *          857,146,382 --> OABORZ
 *          907,546,378 --> KYDQB3
 *          923,665,426 --> SAB4Q3
 * @param i
 * @return
 */
String ltob32( unsigned long i )
{
    unsigned long u = i ;
    String b32("") ;
    char base32StandardAlphabet[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"};
    do
    {
        int d = u % 32 ;
        b32 += base32StandardAlphabet[d];
        u /= 32 ;
    } while( u > 0 );
    return b32 ;
}




/**
 * Get a 2 character string from an 8 bit number.  Numbers >99 will simply return '>>'.
 * @param buffstr
 * @param padding
 * @param value
 */
void get2CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value) {
    char tempstr[3];   //2 + null = 3
    strcpy_P(tempstr, str_EMPTY);
    strcpy_P(buffstr, str_EMPTY);
    itoa(value, tempstr, 10);

    if( value < 10) {                  //0x
        strcat_P(buffstr, padding);
        strcat(buffstr, tempstr);
    } else if ( value < 100)  {        //xx
        strcat(buffstr, tempstr);
    } else {
        strcat_P(buffstr, str_GREATER_THAN);      //>>
    }
}


void get3CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value) {
    char tempstr[4];   //3 + null = 4
    strcpy_P(tempstr, str_EMPTY);
    strcpy_P(buffstr, str_EMPTY);
    itoa(value, tempstr, 10);
    
    if( value < 10) {                  //00x
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
        strcat(buffstr, tempstr);
    } else if ( value < 100)  {        //0xx
        strcat_P(buffstr, padding);
        strcat(buffstr, tempstr);
    } else {
        strcat(buffstr, tempstr);      //xxx
    }  
}


void get4CharStringFrom8BitNumber(char *buffstr, const char *padding, uint8_t value) {
    char tempstr[5];   //4 + null = 5
    strcpy_P(tempstr, str_EMPTY);
    strcpy_P(buffstr, str_EMPTY);
    itoa(value, tempstr, 10);

    if( value < 10) {                  //...x
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
    } else if ( value < 100)  {        //..xx
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
    } else if (value < 1000)  {        // we'll never exceed that for 8-bit anyway
        strcat_P(buffstr, padding);
    }
    strcat(buffstr, tempstr);      //xxxx
}

void get5CharStringFromNumber(char *buffstr, const char *padding, uint16_t value) {
    char tempstr[6];   //5 + null = 6
    strcpy_P(tempstr, str_EMPTY);
    strcpy_P(buffstr, str_EMPTY);
    itoa(value, tempstr, 10);

    if( value < 10) {                  // ....x
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
    } else if ( value < 100)  {        // ...xx
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
    } else if (value < 1000)  {        // ..xxx
        strcat_P(buffstr, padding);
        strcat_P(buffstr, padding);
    } else if (value < 10000) {        // .xxxx
        strcat_P(buffstr, padding);
    }
    strcat(buffstr, tempstr);         // 16-bit is 65535
}
/**
 * Given an 8-bit number, return the minutes/seconds in the format  "m:ss"
 * @param buffstr char array of 5 bytes for holding 4 characters.
 * @param time   integer value representing seconds from 0 - 255 (60 minutes)
 */
void get4CharTimeStringFromSeconds(char *buffstr, uint8_t time) {
    uint8_t min = 0;
    uint8_t sec = 0;
    char tempstr[5];   //4 + null = 5
    strcpy_P(tempstr, str_EMPTY);
    strcpy_P(buffstr, str_EMPTY);

    if (time < 60) { // 0:ss
        itoa(time, tempstr, 10);
        strcat_P(buffstr, str_ZERO);
        strcat_P(buffstr, str_COLON);
        if (time < 10) {
            strcat_P(buffstr, str_ZERO);
        }
        strcat(buffstr, tempstr);
    } else  {        // up to 4:15
        min = time / 60;
        sec = time % 60;
        itoa(min, tempstr, 10);
        strcat(buffstr, tempstr);
        strcat_P(buffstr, str_COLON);
        strcpy_P(tempstr, str_EMPTY);  // clear it so we can use it again
        itoa(sec, tempstr, 10);
        if (sec < 10) {
            strcat_P(buffstr, str_ZERO);
        }
        strcat(buffstr, tempstr);
    }
}

/**
 * Given a 16-bit number, return the minutes/seconds in the format  "mm:ss"
 * @param buffstr char array of 6 bytes for holding 5 characters.
 * @param time   integer value representing seconds from 0 - 6000 (100 minutes)
 */
void get5CharTimeStringFromSeconds(char *buffstr, uint16_t time) {
    uint8_t min = 0;
    uint8_t sec = 0;
    char tempstr[6];   //5 + null = 6
    strcpy_P(tempstr, str_EMPTY);
    strcpy_P(buffstr, str_EMPTY);

    if (time < 60) { //  _0:ss
        itoa(time, tempstr, 10);
        strcat_P(buffstr, str_SPACE);
        strcat_P(buffstr, str_ZERO);
        strcat_P(buffstr, str_COLON);
        if (time < 10) {
            strcat_P(buffstr, str_ZERO);
        }
        strcat(buffstr, tempstr);
    } else if (time < 6000) {        // up to 9:59    _9:59   then    99:59
        min = time / 60;
        sec = time % 60;
        if (min < 10) {
            strcat_P(buffstr, str_SPACE);
        }
        itoa(min, tempstr, 10);
        strcat(buffstr, tempstr);
        strcat_P(buffstr, str_COLON);
        strcpy_P(tempstr, str_EMPTY);  // clear it so we can use it again
        itoa(sec, tempstr, 10);
        if (sec < 10) {
            strcat_P(buffstr, str_ZERO);
        }
        strcat(buffstr, tempstr);
    } else {   // TOO BIG, return :::59 as a warning
        Serial.println(F("TIME > 6000  CANNOT PROPERLY DISPLAY!"));
        sec = time % 60;
        strcat_P(buffstr, str_COLON);
        strcat_P(buffstr, str_COLON);
        strcat_P(buffstr, str_COLON);
        itoa(sec, tempstr, 10);
        if (sec < 10) {
            strcat_P(buffstr, str_ZERO);
        }
        strcat(buffstr, tempstr);
    }
}

