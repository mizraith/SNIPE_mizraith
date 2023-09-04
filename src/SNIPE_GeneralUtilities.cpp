/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */

#include "SNIPE_GeneralUtilities.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "SNIPE_Strings.h"
#include <Wire.h>
#include <avr/pgmspace.h>


void printString_P( const char str[]) {
    char ch;
    if(!str) return;
    while( (ch = pgm_read_byte(str++))) {
        Serial.print(ch);
    }
}



// minihelper
static inline void wiresend(uint8_t x) {
#if ARDUINO >= 100
    Wire.write((uint8_t)x);
#else
    Wire.send(x);
#endif
}

static inline uint8_t wirerecv(void) {
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}



/***************************************************
 ***************************************************
 *   CONVERSION HELPERS
 ***************************************************
 ***************************************************/
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


/**
 *          Add two numbers, and calculate the modulo
 *          of the sum and a third number, M.
 *          In other words, it returns (A+B) % M.
 *          It is designed as a compact mechanism for
 *          incrementing a 'mode' switch and wrapping
 *          around back to 'mode 0' when the switch
 *          goes past the end of the available range.
 *          e.g. if you have seven modes, this switches
 *          to the next one and wraps around if needed:
 *            mode = addmod8( mode, 1, 7);
 *      See 'mod8' for notes on performance.
 *      Thanks to FastLED library under MIT License.
 **/
uint8_t addmod8( uint8_t a, uint8_t b, uint8_t m) {
    //#if defined(__AVR__)
    asm volatile (
        "       add %[a],%[b]    \n\t"
        "L_%=:  sub %[a],%[m]    \n\t"
        "       brcc L_%=        \n\t"
        "       add %[a],%[m]    \n\t"
        : [a] "+r" (a)
        : [b] "r"  (b), [m] "r" (m)
    );

    //straight C implementation
    //a += b;
    //    while( a >= m) a -= m;
    //#endif
    return a;
}



/**
 * Quick add one byte to another, saturating at 0xFF
 * Thanks to FastLED library, under MIT License.
 * @param i - first byte to add
 * @param j - second byte to add
 * @returns the sum of i & j, capped at 0xFF
**/
uint8_t qadd8( uint8_t i, uint8_t j) {
// straight C implementation follows
//    unsigned int t = i + j;
//    if( t > 255) t = 255;
//    return t;

    asm volatile(
    /* First, add j to i, conditioning the C flag */
    "add %0, %1    \n\t"

            /* Now test the C flag.
              If C is clear, we branch around a load of 0xFF into i.
              If C is set, we go ahead and load 0xFF into i.
            */
            "brcc L_%=     \n\t"
            "ldi %0, 0xFF  \n\t"
            "L_%=: "
    : "+a" (i)
    : "a"  (j)
    );

    return i;
}

/**
 * Quick subtract one byte from another, saturating at 0x00
 *  Thanks to FastLED library, under MIT License.
 *  @param i - first byte to subtract from
 *  @param j - second byte to subtract with
 * @returns i - j with a floor of 0
 **/
uint8_t qsub8( uint8_t i, uint8_t j) {
// straight C implementation
//int t = i - j;
//    if( t < 0) t = 0;
//    return t;

    asm volatile(
    /* First, subtract j from i, conditioning the C flag */
    "sub %0, %1    \n\t"

            /* Now test the C flag.
             If C is clear, we branch around a load of 0x00 into i.
             If C is set, we go ahead and load 0x00 into i.
             */
            "brcc L_%=     \n\t"
            "ldi %0, 0x00  \n\t"
            "L_%=: "
    : "+a" (i)
    : "a"  (j)
    );

    return i;
}


/**
 *  Quick calculate an integer average of two unsigned
 *      8-bit integer values (uint8_t).
 *      Fractional results are rounded down, e.g. avg8(20,41) = 30
 *      Thanks to FastLED library, under MIT License.
 **/
uint8_t qavg8( uint8_t i, uint8_t j) {
// Straight C implementation
//  return (i + j) >> 1;

    asm volatile(
    /* First, add j to i, 9th bit overflows into C flag */
    "add %0, %1    \n\t"
            /* Divide by two, moving C flag into high 8th bit */
            "ror %0        \n\t"
    : "+a" (i)
    : "a"  (j)
    );

    return i;
}





/**
 * Get abs() of a signed 8-bit uint8_t
 * Thanks to FastLED library under MIT License
 */
int8_t qabs8( int8_t i)  {
// straight C implementation
//if( i < 0) i = -i;
//    return i;

    asm volatile(
         /* First, check the high bit, and prepare to skip if it's clear */
         "sbrc %0, 7 \n"

         /* Negate the value */
         "neg %0     \n"

         : "+r" (i) : "r" (i)
    );
    return i;
}

//8-bit absolute value function -- check
//the webs for an explanation, but it has
//to do with 2's complement negative notation
int8_t getAbs(int8_t n) {
    int8_t const mask = n >> 7;
    return ((n ^ mask) - mask);
}