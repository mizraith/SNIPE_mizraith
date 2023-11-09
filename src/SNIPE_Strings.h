/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 *
 * Common strings used by pieces of the Neopixel Controller. Note
 * we save RAM by using PROGMEM
 *
 */
 
#ifndef _PPC_Strings_H_
#define _PPC_Strings_H_

#include <Arduino.h>
#include <avr/pgmspace.h>

//*******************************************************************************************
//*******************************************************************************************
//
//  CONSTANT STRINGS
//    save space and progmem by declaring and reusing
//
//  TOOLCHAIN NOTES:
//    Built on Arduino 1.5.8 which uses the new avr-gcc compiler, and changes PROGMEM calls significantly.
//        Old progmem calls:    const prog_car mystring[]  PROGMEM = {"...."}
//        New progmem calls:    const char mystring[]      PROGMEM = {"...."}
//        Old string table calls:   PROGMEM const char * string_table[] = { &s1, &s2...}
//        New string table calls:   PGM_P const string_table[] PROGMEM = { &s1, &s2...}
//*******************************************************************************************
//*******************************************************************************************

#pragma mark STRING CONSTANTS
// CONSTANT CHARS
const char char_CMD = '>';              // expected at start of command, we compare to character
const char char_CR  = '\r';
const char char_LF  = '\n';
const char char_SP  = ' ';
const char char_TB  = '\t';

// CONSTANT STRINGS
// 1 character commands & identifiers
const char str_QUERY [] PROGMEM = "?";    // value query input argument
const char str_ON    [] PROGMEM = "1";    // binary input argument
const char str_OFF   [] PROGMEM = "0";    // binary input argument
//const char str_CMD   [] PROGMEM = ">";    // expected at start of command
const char str_RESP  [] PROGMEM = "@";    // normal response follows
const char str_ERR   [] PROGMEM = "!";    // error response follows
//const char str_CMT   [] PROGMEM = "#";    // comment line follows
const char str_COLON [] PROGMEM = ":";    // sub-token splits on colon
const char str_SPACE [] PROGMEM = " ";    // tokens split on space
const char str_COMMA [] PROGMEM = ",";

// 2 character commands & identifiers
const char str_A    [] PROGMEM = "A";
const char str_A0   [] PROGMEM = "A0";
const char str_A1   [] PROGMEM = "A1";
const char str_A2   [] PROGMEM = "A2";
const char str_A3   [] PROGMEM = "A3";
const char str_D    [] PROGMEM = "D";
const char str_D2   [] PROGMEM = "D2";
const char str_D3   [] PROGMEM = "D3";
const char str_D4   [] PROGMEM = "D4";
const char str_D5   [] PROGMEM = "D5";
const char str_D6   [] PROGMEM = "D6";
//const char str_D7   [] PROGMEM = "D7";
//const char str_D8   [] PROGMEM = "D8";
//const char str_D9   [] PROGMEM = "D9";
const char str_HEX  [] PROGMEM = "0x";   // when hex...

// 3 character commands & identifiers
const char str_SID  [] PROGMEM = "SID";     // Station ID
const char str_TID  [] PROGMEM = "TID";     // Transaction ID
const char str_VER  [] PROGMEM = "VER";     // Version
const char str_ARB  [] PROGMEM = "ARB";     // ARB units
const char str_BIN  [] PROGMEM = "BIN";     // BIN units
const char str_SLP  [] PROGMEM = "SLP";      // Stack Light Percentage
const char str_SLP1  [] PROGMEM = "SLP1";      // Stack Light Percentage
const char str_SLP2  [] PROGMEM = "SLP2";      // Stack Light Percentage
const char str_SLP3  [] PROGMEM = "SLP3";      // Stack Light Percentage
const char str_SLC  [] PROGMEM = "SLC";     // Stack Light Color
const char str_SLC1  [] PROGMEM = "SLC1";     // Stack Light Color
const char str_SLC2  [] PROGMEM = "SLC2";     // Stack Light Color
const char str_SLC3  [] PROGMEM = "SLC3";     // Stack Light Color
const char str_SLM  [] PROGMEM = "SLM";     // Stack Light Mode
const char str_SLM1  [] PROGMEM = "SLM1";     // Stack Light Mode
const char str_SLM2  [] PROGMEM = "SLM2";     // Stack Light Mode
const char str_SLM3  [] PROGMEM = "SLM3";     // Stack Light Mode
const char str_SLX   [] PROGMEM = "SLX";      // Stack Light X  (num LEDs)
const char str_SLX1  [] PROGMEM = "SLX1";     // Stack Light num LEDs
const char str_SLX2  [] PROGMEM = "SLX2";     // Stack Light num LEDs
const char str_SLX3  [] PROGMEM = "SLX3";     // Stack Light num LEDs
const char str_SLINFO[] PROGMEM = "SLINFO";   // Stack Light Info...debugging use only
const char str_BEEP  [] PROGMEM = "BEEP";     // Annoying Beeper
const char str_I2A  [] PROGMEM = "I2A";     // I2C target chip address
const char str_I2S  [] PROGMEM = "I2S";     // I2C target setting
const char str_I2B  [] PROGMEM = "I2B";     // I2C target byte count
const char str_I2W  [] PROGMEM = "I2W";     // I2C Write command
const char str_I2R  [] PROGMEM = "I2R";     // I2C Read command
const char str_I2F  [] PROGMEM = "I2F";     // I2C Find devices on the bus
const char str_RAM  [] PROGMEM = "RAM";     // Checks free RAM
const char str_REBOOT [] PROGMEM = "REBOOT";  // REBOOT COMMAND
const char str_HELP [] PROGMEM = "HELP";    // Send out HELP
// 4 character commands & identifiers
const char str_DESC     [] PROGMEM =    "DESC";     // Description identifier/command
// 5 character commands & identifiers
const char str_BLINK    [] PROGMEM =    "BLINK";    //blink command

// ERROR STRINGS
const int  MAX_ERROR_STRING_LENGTH         =    20;
const char str_VALUE_MISSING    [] PROGMEM =    "VALUE_MISSING";
const char str_VALUE_ERROR      [] PROGMEM =    "VALUE_ERROR";
const char str_Q_REQUIRED       [] PROGMEM =    "?_MISSING";
const char str_ERR_SID_TOO_LONG [] PROGMEM =    "SID_>_30";
const char str_OUT_OF_RANGE     [] PROGMEM =    "OUT_OF_RANGE";
const char str_BYTE_SETTING_ERR [] PROGMEM =    "BYTE_SETTING_ERR";      //16 chars longest message
const char str_NONE_FOUND       [] PROGMEM =    "NONE_FOUND";
const char str_DATA_LENGTH_ERR  [] PROGMEM =    "DATA_LENGTH_ERR";
const char str_UNKNOWN_CMD_ERR  [] PROGMEM =    "UNKNOWN_CMD_ERR";
const char str_INVALID          [] PROGMEM =    "!INVALID_INPUT:";
//const char str_SIDFUN           [] PROGMEM =    "SID";
//char * const str_SIDFUN PROGMEM = "SID";                // this format does NOT work!


// -----------------------------------------------------------------------------
//  unused
// -----------------------------------------------------------------------------
// color and mode constant strings.
const char str_RED              [] PROGMEM =    "RED";
const char str_ORANGE           [] PROGMEM =    "ORANGE";
const char str_YELLOW           [] PROGMEM =    "YELLOW";
const char str_GREEN            [] PROGMEM =    "GREEN";
const char str_AQUA             [] PROGMEM =    "AQUA";
const char str_BLUE             [] PROGMEM =    "BLUE";
const char str_INDIGO           [] PROGMEM =    "INDIGO";
const char str_VIOLET           [] PROGMEM =    "VIOLET";
const char str_WHITE            [] PROGMEM =    "WHITE";
const char str_BLACK            [] PROGMEM =    "BLACK";
const char str_DEFAULT          [] PROGMEM =    "DEFAULT";
const char str_FLASH            [] PROGMEM =    "FLASH";
const char str_PULSE            [] PROGMEM =    "PULSE";
const uint8_t kCOLORLENGTH = 12;   // color names above NEVER get longer than this


const char  str_EMPTY[]              PROGMEM =  {""};
//const char  Space_String[]              PROGMEM =  {" "};
const char  str_ZERO[]               PROGMEM =  {"0"};
//const char  Tab_String[]                PROGMEM =  {"\t"};
//const char  Colon_String[]              PROGMEM =  {":"};
//const char  ModeNum_String[]            PROGMEM =  {"Mode #"};
//const char  One_String[]                PROGMEM =  {"1:"};
//const char  Two_String[]                PROGMEM =  {"   2:"};
//const char  Three_String[]              PROGMEM =  {"   3:"};
const char  str_GREATER_THAN[]        PROGMEM =  {">>"};

//const char  On_String[]                 PROGMEM =  {"ON"};
//const char  Off_String[]                PROGMEM =  {"OFF"};

#endif