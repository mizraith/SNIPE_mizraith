/* **********************************************************************************************
ProjectName:  SNIPE  ( Serial 'n I2C Processing Equipment )
Author:       Red Byer    
Date:         6/28/2023
 ---------------------------------------------------------------------------------------------

# SNIPE_mizraith
**Extensible serial to I2C-and-more tool.  Easily control your Arduino over a comm port. Now with Andon Stack Light Tower type features.  Control some neopixels (slowly) over serial!**

_Major Release:_ 4.0

_Grammar Version:_   2.0

_Changes since 1.0:  Added TID:?, BLINK:? and ECHO:[0:1] commands._

_Changes in 4.0:  Added Stack light commands_.

<!-- TOC -->
* [SNIPE_mizraith](#snipemizraith)
      * [License:](#license)
  * [Purpose:](#purpose-)
  * [Hardware Recommendation:](#hardware-recommendation)
  * [Dependencies](#dependencies)
  * [Serial Protocol:](#serial-protocol)
    * [Messages:](#messages)
    * [Tokens:](#tokens)
      * [Subtoken Order:](#subtoken-order)
      * [Start Characters:](#start-characters)
      * [Values (escaping):](#values-escaping)
      * [Units:](#units)
  * [Identifiers:](#identifiers)
        * [[A0], A1, A2, A3](#a0-a1-a2-a3)
        * [D1, D2, D3, D4, D5, [D6]](#d1-d2-d3-d4-d5-d6-)
        * [ECHO](#echo)
        * [SID](#sid-)
        * [TID](#tid-)
        * [DESC](#desc-)
        * [VER](#ver-)
  * [Commands:](#commands)
    * [Pin Control Commands](#pin-control-commands-)
        * [[A0], A1, A2, A3](#a0-a1-a2-a3-1)
        * [D2, D3, D4, D5, D6](#d2-d3-d4-d5-d6-)
    * [Macro Commands](#macro-commands)
        * [ECHO](#echo-1)
        * [SID](#sid--1)
        * [TID](#tid--1)
        * [DESC](#desc--1)
        * [VER](#ver--1)
        * [BLINK](#blink-)
    * [Stack Light Commands](#stack-light-commands)
      * [SLC1, SLC2, SLC3](#slc1-slc2-slc3)
      * [SLM1, SLM2, SLM3](#slm1-slm2-slm3)
      * [SLA](#sla)
    * [I2C Commands](#i2c-commands)
        * [I2A](#i2a-)
        * [I2S](#i2s-)
        * [I2B](#i2b-)
        * [I2W](#i2w-)
        * [I2R](#i2r-)
        * [I2F](#i2f-)
  * [Sending I2C Commands -- More Detail:](#sending-i2c-commands----more-detail)
  * [Error Messages:](#error-messages)
  * [Test Suite](#test-suite)
<!-- TOC -->

#### License:
<i>The MIT License (MIT)</i>

Copyright (c) 2016, 2019, 2023 Red Byer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.</i>



## Purpose:
SNIPE turns an Arduino into a simple to use lab swiss army
knife instrument, kinda like a poor-man's Labjack.   SNIPE exposes a simple (and easily extended) serial
protocol that allows you to do the following:

1. Communicate with any I2C target
   - Extremely useful as a debug bridge.
   - Can be used to enumerate out all accessible I2C targets.
2. Flip digitial pins on the Arduino.
    - Useful for triggering an interlock or relay.
3. Read raw A2D values from the Arduino.
    - Useful for reading input conditions over serial port.
4. Ping the device
    - Blinks the Arduino so you can figure out which USB device you are talking to.
5. Control neopixel-style LEDs.  *New in 4.0*
    - Control up to 3 sets of LEDs on the digital channels.
    - Full RGB control instead of just an on/off with most indicator lights.
    - Not fast, but good enough for a few times/second (status indicators).


## Hardware Recommendation:
SNIPE is best run on an Arduino with a USB-serial port built in, preferably
one of Arduino's that uses an FTDI chip.  The Nano v3 with a mega328
is a great candidate.

Here's why -- after programming your Arduino with SNIPE, you can
use FTDI's free FTDI_PROG utility to edit the USB device
descriptors and make your device appear as a general
purpose comm port.   At that point, no special drivers are needed and
your end users will NOT have to download and install the Arduino
software just to use SNIPE.   Your SNIPE hardware then becomes a useful
general purpose tool (for <$30 you can replace similar $300 I2C sniffers).


## Dependencies
**mizraith_datetime:** SNIPE currently requires the datetime class.  This class has been extricated
from the marvelous RTClib from Jeelabs and is available as a standalone
arduino library called "mizraith_DateTime".
See: https://github.com/mizraith/mizraith_DateTime

**Arduino EEPROM, Wire:** SNIPE also makes use of the following built-in Arduino libraries: EEPROM, Wire

**Adafruit Neopixel:**  Wonderful library for controlling WS2812/2811 "neopixels" over digital ports.
See: https://github.com/adafruit/Adafruit_NeoPixel


## Serial Protocol:
The serial protocol aims to be readily extended.  It is based on
tokens and subtokens.  Optional elements are kept to a minimum.
For ease of flow control, a start character is used for both
commands and responses.

### Messages:
**Messages** take on the basic form of (up to length 96 chars):

         cTOKEN TOKEN TOKEN .... <CR><LF>

Where "c" is the **start character**, e.g, '>' or '!' or '@' or '#'.

**Tokens** are independent commands sent at once and are seperated by one space.  They are seperated by one space.  This allows a message
to be easily tokenized by the following:

        >>>  message.strip().split(" ")

Where .strip() removes whitespace at the front or end of message that
may have been added.

Note with tokens it is possible to send two or more command tokens
in a given message.  This results in a single reponse line.
Commands are typically processed in the order they are received.
However, with I2C commands there is some token processing
precedence (see Commmands).

_Note that there is no requirement regarding token order._

### Tokens:
**Tokens** take on the following format:

        SUBTOKEN:SUBTOKEN:SUBTOKEN

A token will typically contain 2 or 3 **subtokens**, seperated by a ':'

#### Subtoken Order:
The order of subtokens is generally agreed to be one of the following:

        <command>:<value>
        <command>:<value>:<units> (shorter form is preferred)
        <command>:<value>:<optionalvalue>  *new in 4.0, used in stacklights*
        <identifier>:<value>   (discouraged for longer form)
        <identifier>:<value>:<units>

#### Start Characters:
        '>' is used to **start a normal command** to SNIPE
        '#' is used to significy a **non-value respone**, like a comment or header string
        '@' is used for **value response**
        '!' is used for **error responses**

#### Values (escaping):
Because values headed to the I2C port might contain <CR> or <LF> or 0
as a byte, the data is **escaped** by sending it as a hex string, e.g.:

        <command>:0x0AF1

Naturally, certain I2C commands default to interpreting the input string
as hex.  As a result, the '0x' is required gramatically.  The return
response will also be escaped in a similar matter.

#### Units:
**Units** are more of a convenience **subtoken** (a GUI may use them to
label fields), but may be used for parsing commands.
Depending on the command/identifier, SNIPE uses the following units.

            BIN     binary
            ARB     arbitrary
            HEX     hexadecimal value


## Identifiers:
Following is a list of **identifiers** that this version of SNIPE supports.
Identifiers are returned in ALL CAPS, but should be considered
case -insensitive-.

##### [A0], A1, A2, A3
- **description:**  Analog 0:3 pin readback
- **value range:**  0 - 1024
- **units:**        ARB
- **example:**      `@A0:254:ARB`
- _Note:  A0 is the recommended standard analog input._

##### D1, D2, D3, D4, D5, [D6]
- **description:**  Digital 6 readback
- **value range:**  0 : 1
- **units:**        BIN
- **example:**      `@D6:0:BIN`
- _Note:     D6 is the recommended standard digital output since it is not used for much on the Arduino._

##### ECHO
- **description:**  Turn response echo on/off
- **value range:**  0:1    Default is 0
- **units:**        BIN
- **example:**
  - command:  `>ECHO:1`
  - response: `# RECEIVED_INPUT->ECHO:1`
  - response: `@ECHO:1`
  - command:  `>ECHO:0`
  - response: `@ECHO:0`

##### SID
- **description:**  Station ID
- **value range:**  string value set by user, can NOT contain spaces
- **example:**      `SID:My_Uno_3`

##### TID
- **description:** Transaction ID.  If the command contains a TID, then SNIPE will return a response with the same TID.
- **value range:**  String, less than 8 characters please
- **example:**
  - command:  `>A0:? TID:1a35`
  - response: `@A0:245 TID:1a35`

##### DESC
- **description:**  Description of our device
- **value range:**  string, < 32 ASCII chars without spaces
- **example:**      `DESC:SNIPE_FOR_ARDUINO`

##### VER
- **description:**  Version
- **value range:**  integer number as string  0:999
- **example:**      `VER:24`


## Commands:
The following is a complete listing of **commands** this version of
SNIPE supports.   Commands are shown in all caps and are currently
case -insensitive- but this could change.

### Pin Control Commands
##### [A0], A1, A2, A3
- **description:**     Get the Analog input
- **input argument:**  ?
- **value range:**     0 - 1024
- **units:**           ARB
- **example:**
  - command:  `>A0:?`
  - response: `@A0:254:ARB`
- _Note: A0 is the recommended standard analog input._

##### D2, D3, D4, D5, D6
- **description:**     Set or Get the digital pin value
- **input argument:**  ?    or   0:1
- **value range:**     0 : 1
- **units:**           BIN
- **example:**
  - command:  `>D6:?`
  - response: `@D6:0:BIN`
  - command:  `>D6:1`
  - response: `@D6:1:BIN`
- _Note:     D6 is the recommended standard digital output since it is not used for much on the Arduino._

### Macro Commands
##### ECHO
- **description:**  Turn response echo on/off.  Results in a 2-line response, but is useful for debugging.
- **input argument:**  0:1  or ?   Default is 0
- **units:**        BIN
- **example:**
  - command:  `>ECHO:?`
  - response: `@ECHO:0:BIN`
  - command:  `>ECHO:1`
  - response: `@ECHO:1:BIN`  (echoing applies for next command)
  - command:  `>ECHO:?`
  - response: `# RECEIVED_INPUT->ECHO:?`
  - response: `@ECHO:1:BIN`

##### SID
- **description:**     Station ID
- **input argument:**  ?   or  stationID string, no spaces allowed
- **value range:**     string value set by user, can NOT contain spaces
- **example:**
  - command:  `>SID:?`
  - response: `@SID:None_Set`
  - command:  `>SID:MyUno_03`
  - response: `@SID:MyUno_03`

##### TID
- **description:**     Transaction IDs can be included in messages. They will be echoed back in the associated resopnse.
                       The intent is to support synchronizing commands and responses.
                       Querying the TID will return the last TID value.
- **input argument:**  ? or string, less than 8 characters, no spaces please
- **example:**
  - command:  `>A0:? TID:1a35`
  - response: `@A0:245 TID:1a35`
  - command:  `>TID:?`
  - response: `@TID:1a35`

##### DESC
- **description:**     Description of our device
- **input argument:**  ?   <grammatically required>
- **value range:**     string, < 32 ASCII chars without spaces
- **example:**
  - command:  `>DESC:?`
  - response: `@DESC:SNIPE_FOR_ARDUINO`
  - command:  `>DESC:? VER:?`
  - response: `@DESC:SNIPE_FOR_ARDUINO VER:012`

##### VER
- **description:**     Version
- **input argument:**  ?
- **value range:**     integer number as string  0:999
- **example:**
  - command:  `>VER:?`
  - response: `@VER:012`

##### BLINK
- **description:**     Blinks the Arduino onboard LED (D13 typically)
- **input argument:**  ? or 0:5000 Number of milliseconds to blink for
- **example:**
  - command:  `>BLINK:5000`
  - response: `@BLINK:5000`
  - command:  `>BLINK:?`
  - response: `@BLINK:3502`

### Stack Light Commands
_SLC commands use [D7] and [D8] and [D9] by default on a nano._
#### SLC1, SLC2, SLC3
- **description:** Stack Light Color.  Sets the stack light color.
- **input argument:** ? or color value as a hexadecimal string. 0xRRGGBB. The leading '0x' is required. If not properly formatted, will be ignored.  Setting to 0x0 will turn the color "black", or off.
  **example:**
  - command: `>SLC1:0xFF0000`
  - response:`@SLC1:0xFF0000`   The LED on STack Light 1 is now RED.
  - command: `>SLC1:0x0`
  - response: `@SLC1:0x0`    The LED is now off.
- _NOTE:  **SLC1** --> **[D7]**, **SLC2** -->**[D8]**, **SLC3** --> **[D9]**_

#### SLM1, SLM2, SLM3
- **description:**  Stack Light Mode.  Sets the stack light mode for the target.
- **input argument:** ? or integer argument.
- **input values:**
  - 0:  Off (Same as `SLC:0x0` but retains set color.)
  - 1:  On, Steady State
  - 2:  On, Pulsing.  Option subtoken for full-cycle blink rate in ms.
  - 3:  On, Flashing.  Optional subtoken for full-cycle blink rate in ms.
- **example:**
  - command: `>SLM1:1`
  - response:`@SLM1:1`   The mode for #1 is ON, Steady State.
  - command: `>SLM1:3:500`  The mode is now flashing every 500ms
  - response: `@SLM1:3:500`
  - command: `>SLM1:0`
  - response: `@SLM1:0`  The light for stack light #1 is off

#### SLA
- **description:**  Stack Light alarm.  Convenience method for setting digital pin high to activate annoying beeper.
- **input argument:** ? or binary argument.
- **input values:**  0 : 1. 0 = Alarm Off.  1 = Alarm On.
- _NOTE:  SLA will be wired to [A6] as a digital output._

### I2C Commands
_I2C commands use SCL and SDA pins.  This is [A4] and [A5] on a nano._
##### I2A
- **description:**     I2C target chip address
- **input argument:**  ?  for last setting
                       0:127   (8 bit value as a decimal string)
- **example:**
  - command:  `>I2A:125`
  - response: `@I2A:125`
  - command:  `>I2A:?`
  - response: `@I2A:125`

##### I2S
- **description:**     I2C target setting (byte or register address)
- **input argument:**  ?  for last setting
                       0:255 (8 bit value as a decimal string)
- **example:**
  - command:  `>I2S:17`
  - response: `@I2S:17`
  - command:  `>I2S:?`
  - response: `@I2S:17`

##### I2B
- **description:**     I2C number of bytes to send or receive
- **input argument:**  ? for last setting
                       0:16  (number of bytes as a decimal string)
- **example:**
  - command:  `>I2B:2`
  - response: `@I2B:2`
  - command:  `>I2B:?`
  - response: `@I2B:2`

##### I2W
- **description:**     I2C write to currently set address, setting with byte count.
- **input argument:**  data bytes as hexadecimal string.
                       the leading 0x is required.
                       If not enough bytes are provided (per I2B)
                       then the write will fail.   Similarly, if
                       too may bytes are provided, the write will fail.
- _NOTE:         THIS will be processed after all other items in the message, but before an I2R._
- **example:**
  - command:  `>I2A:110 I2S:23 I2B:2 I2W:0xA0F3 TID:14397`
  - response: `@I2A:110 I2S:23 I2B:2 I2W:0xA0F3 TID:14397`
  - command:  `>I2W:0xA1F4`    (resend to same chip/addr)
  - response: `@I2W:0xA1F4`
  - command:  `>I2W:0xA2F3FF TID:14399`  (resend..too many bytes`
  - response: `!I2W:BYTE_SETTING_ERR TID:14399`
  - command:  `>I2B:?`
  - response: `@I2B:2`
  - command:  `>I2B:3`
  - response: `@I2B:3`
  - command:  `>I2W:0xA2F3FF`     (now we have the right # bytes)
  - response: `@I2W:0xA2F3FF`

##### I2R
- **description:**     I2C read for currently set address, setting, bytes
- **input argument:**  ?    <grammatically required>
- _NOTE:  THIS will be processed after all other items in the message, and after I2W._
- **example:**
  - command:  `>I2A:110 I2S:23 I2B:2 I2R:? TID:14397`
  - response: `@I2A:110 I2S:23 I2B:2 I2R:0xA0F3 TID:14397`
  - command:  `>I2R:?`      (re-read from same chip/addr)
  - response: `@I2R:0xA0F3`
  - command:  `>I2A:1 I2R:? TID:14399`
  - response  `!I2A:1 I2R:0x00`      (inccorrect address?)

##### I2F
- **description:**     Get a listing of chips on the I2C bus
- **input argument:**  ?      <gramatically required>
- **example:**
  - command:  `>I2F:?`
  - response: `@I2F:1,12,23,113`


## Sending I2C Commands -- More Detail:
SNIPE attempts to be as flexible as possible in constructing I2C
messages and reading/writing them.

The grammar allows for you to break up I2C commands into multiple messages

            command: >I2A:110       response:  @I2A:110
            command: >I2S:32        response:  @I2S:32
            command: >I2B:1         response:  @I2B:1
            command: >I2W:0xA2      response:  @I2W:0xA2

Or you can put it all together as one logically flowing message

            command:  >I2A:110 I2S:32 I2B:1 I2W:0xA2 TID:123
            response: @I2A:110 I2S:32 I2B:1 I2W:0xA2 TID:123

And you can alter the order as well because I2C Writes
are processed near last and I2C Reads are processed dead last.

The following would first set the address, setting, bytes, the write
the data, then read the data back.  Note the return string is
formatted in order of operation.

            command:  >I2W:0xA3 I2R:? I2A:110 I2S:32 I2B:1 TID:155
            response: @I2A:110 I2S:32 I2B:1 I2W:0xA3 I2R:0xA3 TID:155


## Error Messages:
Because SNIPE is operating on a limited microcontroller, error
messages will be brief, all caps, and without spaces.  The error
message protocol is to be developed, but always starts with a '!'
and will attempt to echo back the command that generated the error.

The grammar currently utilizes the following error messages:

            VALUE_MISSING
            VALUE_ERROR
            ?_MISSING
            SID_>_30
            OUT_OF_RANGE
            BYTE_SETTING_ERR
            NONE_FOUND
            DATA_LENGTH_ERR
            SID

## Test Suite
SNIPE comes with a python script for testing the full communications
protocol.  This script is in the main directory as SNIPE_tests.py.
The script utilizes python 2.7 and pyserial to do the work. Should
you wish to extend or fork SNIPE, you will want to run the test
suite to catch any subtle formatting bugs.

To use the test suite, first load up your Arduino with SNIPE and
make a note of the comm port (e.g. /dev/tty.usbmodem1411).

From your command line, launch the python script as main with
the port as the first argument, e.g.:

            python SNIPE_tests.py /dev/tty.usbmodem1411

Sit back and watch python hammer your little Arduino and verify
that the comm functions and grammar are all correct.  The script
provides a full printout of the comm transactions to aid in
debugging.~~


##### CHANGELOG:
- 12/22/2015   Original base code written
- 12/28/2015   Initial release of limited version, grammar 0.1
- 1/27/2016    Grammar improvements, removed 'get' command, spec'd I2C
- 2/12/2016    Fully implemented with a python test suite and nearly 100% coverage.
- 6/11/2016    Updated readme with dependencies section.
- 5/2019       Stability Edits
- 6/2023       UPDATE 4.0  includes StackLight commands for Neopixel RGB LED control.

*************************************************************************** */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
// Uses SCL and SDA pins
#include <Wire.h>
// included for the datetime class.  Wire.h makes the RTCLib happy.
#include"mizraith_DateTime.h"   

#pragma mark DEBUG MODE
// CLion devs edit in CMakeLists.txt and leave below commented out
//  Arduino developers select one
// #define DEBUG_MODE 1
// #define DEBUG_MODE 0
// CLION -- can use compiler flag to set a bool, at the expense of RAM but tighter looking code
// PlatformIO users, use:  build_flags = -D DEBUG_MODE=1

#pragma mark LED LIB IMPORT
// *******************************************************************************************************
// UNCOMMENT ONE OF THE FOLLOWING TO CHOOSE WITH LEDS YOU ARE WORKING WITH
//    The Adafruit Neopixel library supports LEDs with WS2811 and WS2812 chips (aka "neopixels")
//    The Adafruit WS2801 library supports WS2801's.  usLEDwarehouse sells these.
//
// DEVELOPER NOTES: USING #defines does not work in CLion under CMake.
//    !!! CLion / CMake users -- these #defines are commented out and the corresponding flags
//      must be set in the CMakeLists.txt file.
//    !!!Arduino IDE developers:
//      Because of how Arduino links, I've had to put a duplicate #define in other locations.
//     YOU MUST MATCH SETTING IN:
//                                PPC_Modes.h                   <line 12>
//
// ARDUINO DEVELOPERS SELECT ONE:
// NEOPIXELs are build on the board itself, these are the adafruit WS2811 LEDs
//#define USE_NEOPIXEL_LEDS
// WS2801 is what we will use in production, the underwater larger pixels.
//define USE_WS2801_LEDS

//now let the defines do their work
#ifdef USE_NEOPIXEL_LEDS
//#include "/Users/red/Documents/Computer_Science/ARDUINO_DEVELOPMENT/libraries/Adafruit_Neopixel/Adafruit_NeoPixel.h"
    #include "Adafruit_NeoPixel.h"
#endif

#ifdef USE_WS2801_LEDS
#include "Adafruit_WS2801.h"
#endif


#pragma mark FORWARD DECLARATIONS
// ********************************************************************************************
//extern void setup();
//extern void loop();
// SETUP
void blinky_worker();
// SERIAL INTERRUPT FUNCTION
void serialEvent();
// INPUT STRING PROCESSING
void handleInputString();
void handleToken(char *);
void printSubTokenArray();
// COMMAND HANDLING
void handle_A0();
void handle_A1();
void handle_A2();
void handle_A3();
void handle_A_worker(uint8_t);
void handle_D2();
void handle_D3();
void handle_D4();
void handle_D5();
void handle_D6();
void handle_D_worker(uint8_t );
void handle_SID();
void handle_TID();
void handle_VER();
void handle_DESC();
void handle_SLC1();
void handle_SLC2();
void handle_SLC3();
void handle_SLC_worker(uint8_t);
void handle_SLM1();
void handle_SLM2();
void handle_SLM3();
void handle_SLM_worker(uint8_t);
void handle_SLA();
void handle_I2A();
void handle_I2B();
void handle_I2W();
void handle_I2R();
void handle_I2S();
void handle_I2F();
void handle_BLINK();
// SETUP HELPERS
void serialPrintHeaderString();
void printString_P(const char *);
void printSerialInputInstructions();
void printSerialDataStart();
// EEPROM HELPERS
uint8_t isVirginEEPROM();
void initEEPROM(int, uint8_t);
void readSIDFromEEPROM();
void writeSIDToEEPROM();
// LOW LEVEL HELPERS
void convertHexStringToByteArray(char *, uint8_t *);
void convertByteArrayToHexString(byte *, uint8_t, char *);
byte getHexNibbleFromChar(char);
char getCharFromHexNibble(uint8_t);
void printBytesAsDec(uint8_t*, uint8_t);
// I2C HELPERS
void wiresend(uint8_t);
uint8_t wirereceive();
void perform_I2C_write();
int perform_I2C_write_error();
void perform_I2C_read();
// RAM HELPERS
void checkRAMandExitIfLow(uint8_t);
void checkRAM();
int freeRam();


const DateTime COMPILED_ON = DateTime(__DATE__, __TIME__);
const String CURRENT_VERSION = "003";
const String DESCRIPTION = "SNIPE_for_Arduino";

#pragma mark Pinouts
// PIN OUTS
#define ANALOG_INPUT   A0
#define LED_PIN        13
#define RELAY_PIN      6
#define SL1_PIN        7
#define SL2_PIN        8
#define SL3_PIN        9
#define ALARM_PIN      A6

#pragma mark Constants
// CONSTANT CHARS
const char char_CMD = '>';
const char char_CR  = '\r';
const char char_LF  = '\n';

// CONSTANT STRINGS
// 1 character commands & identifiers
const char str_QUERY [] PROGMEM = "?";    // value query input argument
const char str_ON    [] PROGMEM = "1";    // binary input argument
const char str_OFF   [] PROGMEM = "0";    // binary input argument
const char str_CMD   [] PROGMEM = ">";    // expected at start of command
const char str_RESP  [] PROGMEM = "@";    // normal response follows
const char str_ERR   [] PROGMEM = "!";    // error response follows
const char str_CMT   [] PROGMEM = "#";    // comment line follows
const char str_COLON [] PROGMEM = ":";    // sub-token splits on colon
const char str_SPACE [] PROGMEM = " ";    // tokens split on space
const char str_COMMA [] PROGMEM = ",";


// 2 character commands & identifiers
const char str_A0   [] PROGMEM = "A0";
const char str_A1   [] PROGMEM = "A1";
const char str_A2   [] PROGMEM = "A2";
const char str_A3   [] PROGMEM = "A3";
const char str_D2   [] PROGMEM = "D2";
const char str_D3   [] PROGMEM = "D3";
const char str_D4   [] PROGMEM = "D4";
const char str_D5   [] PROGMEM = "D5";
const char str_D6   [] PROGMEM = "D6";
const char str_D7   [] PROGMEM = "D7";
const char str_D8   [] PROGMEM = "D8";
const char str_D9   [] PROGMEM = "D9";
const char str_HEX  [] PROGMEM = "0x";   // when hex...

// 3 character commands & identifiers
const char str_SID  [] PROGMEM = "SID";     // Station ID
const char str_TID  [] PROGMEM = "TID";     // Transaction ID
const char str_VER  [] PROGMEM = "VER";     // Version
const char str_ARB  [] PROGMEM = "ARB";     // ARB units
const char str_BIN  [] PROGMEM = "BIN";     // BIN units
const char str_SLC  [] PROGMEM = "SLC";     // Stack Light Color
const char str_SLM  [] PROGMEM = "SLM";     // Stack Light Mode
const char str_SLA  [] PROGMEM = "SLA";     // Stack Light Alarm
const char str_I2A  [] PROGMEM = "I2A";     // I2C target chip address
const char str_I2S  [] PROGMEM = "I2S";     // I2C target setting
const char str_I2B  [] PROGMEM = "I2B";     // I2C target byte count
const char str_I2W  [] PROGMEM = "I2W";     // I2C Write command
const char str_I2R  [] PROGMEM = "I2R";     // I2C Read command
const char str_I2F  [] PROGMEM = "I2F";     // I2C Find devices on the bus
const char str_RAM  [] PROGMEM = "RAM";     // Checks free RAM
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
const char str_SIDFUN           [] PROGMEM =    "SID";
//char * const str_SIDFUN PROGMEM = "SID";                // this format does NOT work!

#pragma mark String Lengths
// STRING LENGTH & EEPROM LOCATION CONSTANTS
const int MAX_INPUT_LENGTH  = 64;
const int MAX_OUTPUT_LENGTH = 96;
const int SID_MAX_LENGTH    = 30;   //24 + null
const int SID_EEPROM_START_ADDRESS = 0;
const int MAX_NUMBER_TOKENS = 10;

#pragma mark Variable Strings
// VARIABLE STRINGS
char SID[SID_MAX_LENGTH] = "USE_>SID:xxx_TO_SET";
char input_buffer[MAX_INPUT_LENGTH];       // buffer to accumulate our input string into
char input_string[MAX_INPUT_LENGTH];       // copy of input_buffer for processing
//char output_string[MAX_OUTPUT_LENGTH];   // buffer for building our output string
char * subtokens[MAX_NUMBER_TOKENS];       // Yes, an array of char* pointers.  We should never have 10 subtokens!
String output_string = "";                 // might as well use the helper libraries supplied by arduino
//char transaction_ID_string[9];           // Transaction ID, max 8 chars

#pragma mark I2C Variables
// Variables for I2C
uint8_t I2C_Address = 0;
uint8_t I2C_Register = 0;
uint8_t I2C_Bytes    = 0;
uint8_t I2C_Data[32];       // set large for init...

#pragma mark Analog Variables
// Analog Input Variables
uint16_t A0_value = 0;
uint16_t A1_value = 0;
uint16_t A2_value = 0;
uint16_t A3_value = 0;

#pragma mark Message Processing Variables
// Message Processing Variables
boolean processing_is_ok = true;
//boolean TID_is_present = false;

#pragma mark State Variables
// Serial Interrupt Variable
volatile boolean input_string_ready = false;

#pragma mark Blink Variables
uint32_t blink_time = 0;
unsigned long blink_start = 0;
unsigned long last_blink_change = 0;
boolean is_blinking = false;
const int blink_toggle_time_ms = 250;




#pragma mark Setup & Main
/***************************************************
 ***************************************************
 *   SETUP 
 ***************************************************
 ***************************************************/

void setup() {
    Serial.begin(57600);
    Wire.begin();

    if (isVirginEEPROM()) {
        Serial.println(F("# Setup: Init'ing EEPROM"));
        initEEPROM(1024, 0x00);
        Serial.print("# Setup: Will init SID to: " );
        Serial.println(SID);
        writeSIDToEEPROM();      // write out the default SID
    }
    readSIDFromEEPROM();

    // Developer note: This next line is key to prevent our output_string (String class)
    // from growing in size over time and fragmenting the heap.  By calling this now
    // we end up saving about 50-60bytes of heap fragmentation.
    output_string.reserve(MAX_OUTPUT_LENGTH);

    serialPrintHeaderString();
    checkRAMandExitIfLow(0);

    pinMode(ANALOG_INPUT, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(ALARM_PIN,OUTPUT);
    pinMode(2,  OUTPUT);
    pinMode(3,  OUTPUT);
    pinMode(4,  OUTPUT);
    pinMode(5,  OUTPUT);
    pinMode(6,  OUTPUT);
    pinMode(SL1_PIN, OUTPUT);
    pinMode(SL2_PIN, OUTPUT);
    pinMode(SL3_PIN,OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(ALARM_PIN, LOW);
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(LED_PIN, LOW);

    printSerialInputInstructions();
    printSerialDataStart();
}


/***************************************************
 ***************************************************
 *   MAIN LOOP 
 ***************************************************
 ***************************************************/
void loop() {
    // put your main code here, to run repeatedly:
    A0_value = analogRead(A0);
    A1_value = analogRead(A1);
    A2_value = analogRead(A2);
    A3_value = analogRead(A3);

    if (input_string_ready) {
        handleInputString();
        input_string_ready = false;
    }

    if (is_blinking) {
        blinky_worker();
    }

    //delay(15);
}



void blinky_worker() {
    unsigned long current_time = millis();
    if (current_time - blink_start > blink_time) {
        digitalWrite(LED_PIN, LOW);
        is_blinking = false;
    } else if (current_time - last_blink_change > blink_toggle_time_ms) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        last_blink_change = millis();
    }
}





#pragma mark Serial Interrupt Function
/***************************************************
 ***************************************************
//  SERIAL INPUT FUNCTION
//     ACTS IMMEDIATELY ONE CHAR AT A TIME TO
//     BUILD UP THE INPUT STRING
//     UNTIL IT SEES A CARRIAGE RETURN OR LINE FEED
 ***************************************************
 ***************************************************/
char c;
const char NONE = -1;
bool accumulatestring = false;

void serialEvent() {
    c = Serial.read();
    if (  (c == char_CR ) ||
          (c == char_LF ) )  {
        c = NONE;
        if(accumulatestring) {
            accumulatestring = false;
            strcpy(input_string, "");  // blank it
            strcpy(input_string, input_buffer);  // back it up, then send to process
            input_string_ready = true;
            return;
        } else {
            return;  //we're not accumulating bare newlines, so just ignore
        }
    }

    if( c == char_CMD ) {
        accumulatestring = true;
        strcpy(input_buffer, "");  //blank it to accumulate
    }

    if(accumulatestring) {
        if ( c == char_CMD ) {
            return;  //don't accumulate start char
        }  else {
            if (strlen(input_buffer) > (MAX_INPUT_LENGTH - 2) ) {
                return;               //too long, don't append anymore
            } else {
                char cbuff[2] = " ";  //char plus null0
                cbuff[0] = c;
                strcat(input_buffer,  cbuff);
            }
        }
//    } else if (c != NONE) {
//      //SEE IF USER ENTERED A MODE NUMBER
//      int x = c - 48;     //simple c to i conversion minus 1 for array indexing :  ord for 0 is 48. 
//      if ( (x >= mizraith_DigitalClock::INPUT_EVENT_OFFSET ) && (x < mizraith_DigitalClock::MAX_NUMBER_OF_INPUT_EVENTS ) ) {
//        input_event = x;
//        input_event_data = 0;
//        Serial.print(F("Input Event Via Serial: "));
//        Serial.println(x);
    } else {
        // we're not accumulating but getting characters...yuck!
        Serial.print(F("!Invalid_input:"));
        Serial.print(c);
        Serial.println(F(":Commands_start_with_>"));
    }
    c = NONE;       //reset
}



#pragma mark Input String Processing
/***************************************************
 ***************************************************
//  MAIN INPUT HANDLING FUNCTIONS
 ***************************************************
 ***************************************************/
void handleInputString() {
    //checkRAMandExitIfLow(1);
    //long timestart = millis();

    //Note that the ">" has already been removed by this point.
    processing_is_ok = true;  // leave as true....only set false if we encounter an error

    //Serial.print(F("# RECEIVED_INPUT->"));
    //Serial.println(input_string);

    char* token;
    char* tempstring;
    char* stringtofree;

    output_string = "";                 // clear it

    tempstring = strdup(input_string);  // does arduino support strdup?
    if (tempstring != NULL) {
        stringtofree = tempstring;    //store copy for later release

        char space[2];
        strcpy_P(space, str_SPACE);
        // *** SPLIT INTO MAIN TOKENS ****
        while ((token = strsep(&tempstring, space)) != NULL)  {
            //if we get 2 spaces together we get a 0-length token and we should ignore it
            if ( ( token != NULL ) && (strlen(token) != 0 )  ) {
                //Serial.println(token);
                handleToken(token);
            }
        }  // end main token handling
        free(stringtofree);
    }

    // SEND OUT OUR RESULT
    if (!processing_is_ok) {
        printString_P(str_ERR);
    } else {
        printString_P(str_RESP);
    }
    Serial.println(output_string);
    // clean up
    output_string = "";
    free(tempstring);

    //long timeend = millis();
    //long totaltime = timeend - timestart;
    //Serial.print(F("# Total execution (ms): "));
    //Serial.println(totaltime, DEC);
    //checkRAMandExitIfLow(11);
    return;
}


void handleToken(char* ctoken) {
    //checkRAMandExitIfLow(2);
    char *temptoken;
    char *tokentofree;
    char *subtoken;

    boolean gotWrite = false;
    boolean gotRead = false;

    char colon[2];
    strcpy_P(colon, str_COLON);

    temptoken = strdup(ctoken);
    tokentofree = temptoken;         // keep a pointer around for later use
    // SPLIT INTO SUBTOKENS AND STORE IN OUR GLOBAL ARRAY so we don't have to do the work again
    uint8_t i = 0;
    while ((subtoken = strsep(&temptoken, colon)) != NULL) {
        if (subtoken != NULL) {    // a 0-length subtoken _is_ acceptable...maybe there is no value.
            //Serial.print(F("    "));
            //Serial.println(subtoken);
            subtokens[i] = subtoken;
        }
        i++;
    }  // end subtoken split
    subtokens[i] = NULL;

    //checkRAMandExitIfLow(22);

    // make cmd into uppercase so that our commands are case insensitive  // TODO: VERIFY THIS WORKS
    i = 0;
    while (subtokens[0][i])
    {
        c = subtokens[0][i];
        subtokens[0][i] = toupper(c);
        i++;
    }

    if        (strcmp_P(subtokens[0], str_A0) == 0) {
        handle_A0();
    } else if (strcmp_P(subtokens[0], str_A1) == 0)  {
        handle_A1();
    } else if (strcmp_P(subtokens[0], str_A2) == 0)  {
        handle_A2();
    } else if (strcmp_P(subtokens[0], str_A3) == 0)  {
        handle_A3();
    } else if (strcmp_P(subtokens[0], str_D2) == 0)  {
        handle_D2();
    } else if (strcmp_P(subtokens[0], str_D3) == 0)  {
        handle_D3();
    } else if (strcmp_P(subtokens[0], str_D4) == 0)  {
        handle_D4();
    } else if (strcmp_P(subtokens[0], str_D5) == 0) {
        handle_D5();
    } else if (strcmp_P(subtokens[0], str_D6) == 0) {
        handle_D6();
    } else if (strcmp_P(subtokens[0], str_SID) == 0) {     // could use (0 == strcmp(subtokens[0], "SID")) (strcmp_P(subtokens[0], PSTR("SID")) == 0 ) (cmd.equalsIgnoreCase(str_SID))
        handle_SID();
    } else if (strcmp_P(subtokens[0], str_TID) == 0) {
        handle_TID();
    } else if (strcmp_P(subtokens[0], str_VER) == 0) {
        handle_VER();
    } else if (strcmp_P(subtokens[0], str_I2A) == 0) {
        handle_I2A();
    } else if (strcmp_P(subtokens[0], str_I2S) == 0) {
        handle_I2S();
    } else if (strcmp_P(subtokens[0], str_I2B) == 0) {
        handle_I2B();
    } else if (strcmp_P(subtokens[0], str_I2W) == 0) {
        gotWrite = true;
    } else if (strcmp_P(subtokens[0], str_I2R) == 0) {
        gotRead = true;
    } else if (strcmp_P(subtokens[0], str_I2F) == 0) {
        handle_I2F();
    } else if (strcmp_P(subtokens[0], str_DESC) == 0) {
        handle_DESC();
    } else if (strcmp_P(subtokens[0], str_BLINK) == 0) {
        handle_BLINK();
    } else if (strcmp_P(subtokens[0], str_RAM) == 0) {
        checkRAM();
    } else {
        processing_is_ok = false;
        // add bad response to outputstring
    }
    // Delayed commands, processed with stated precedence.
    if (gotWrite) {
        handle_I2W();
        gotWrite = false;
    }
    if (gotRead) {
        handle_I2R();
        gotRead = false;
    }
    // clean it up
    free(tokentofree);
    free(temptoken);
    free(subtoken);
    //checkRAMandExitIfLow(222);
}


void printSubTokenArray() {
    int i = 0;
    Serial.println(F("# SubTokens: "));
    while (subtokens[i] != NULL) {
        if(i == 0) {
            Serial.print(F("..CMD: "));
        } else {
            Serial.print(F(".."));
            Serial.print(F("i="));
            Serial.print(i, DEC);
            Serial.print(F(": "));
        }
        Serial.println(subtokens[i]);
        i++;
    }
}




#pragma mark COMMAND HANDLER EXPLANATION
/**
 * Command handlers must all share the same philosophy.
 * READS FROM:  the subtokens array  (pointers to char* strings)
 *               e.g.  subtokens[0] = "TID"   subtokens[1] = "1234" subtokens[2]=NULL
 * IMPACTS:  processing_is_ok   sets to false if something goes wrong
 * APPENDS:  its results to output_string
 */
/***************************************************
 ***************************************************
//  COMMAND HANDLING FUNCTIONS
 ***************************************************
 ***************************************************/
#pragma mark Command Handling
void handle_A0() {
    handle_A_worker(0);
    return;
}

void handle_A1() {
    handle_A_worker(1);
    return;
}

void handle_A2() {
    handle_A_worker(2);
    return;
}

void handle_A3() {
    handle_A_worker(3);
    return;
}

/**
 * function: handle_A_worker
 * numA:   Arduino analog input
 * appends:  analog read port value to output string
 */
void handle_A_worker(uint8_t numA) {
    char buff[10];
    String resp("");
    String port("");
    uint16_t val = 0;
    switch(numA) {
        case 0:
            strcpy_P(buff, str_A0);
            port = buff;
            val = A0_value;
            break;
        case 1:
            strcpy_P(buff, str_A1);
            port = buff;
            val = A1_value;
            break;
        case 2:
            strcpy_P(buff, str_A2);
            port = buff;
            val = A2_value;
            break;
        case 3:
            strcpy_P(buff, str_A3);
            port = buff;
            val = A3_value;
            break;
        default:
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_VALUE_ERROR);
            port = err;
            val = 0;
            break;
    }
    resp = port;
    strcpy_P(buff, str_COLON);
    resp += buff;

    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += val;                  // Arduino String allows concat of an int, but must be on it's own line
        strcpy_P(buff, str_COLON);
        resp += buff;
        strcpy_P(buff, str_ARB);
        resp += buff;
    } else {
        processing_is_ok=false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err , str_Q_REQUIRED);
        resp +=  err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}



void handle_D2() {
    handle_D_worker(2);
    return;
}

void handle_D3() {
    handle_D_worker(3);
    return;
}

void handle_D4() {
    handle_D_worker(4);
    return;
}

void handle_D5() {
    handle_D_worker(5);
    return;
}

void handle_D6() {
    handle_D_worker(6);
    return;
}


/**
 * function: handle_D_worker
 * numpin:   Arduino digital input
 * appends:  digital pin status to output display
 */
void handle_D_worker(uint8_t numpin) {
    char buff[10];
    String resp("");
    switch(numpin) {
        case 2:
            strcpy_P(buff, str_D2);
            resp = buff;
            break;
        case 3:
            strcpy_P(buff, str_D3);
            resp = buff;
            break;
        case 4:
            strcpy_P(buff, str_D4);
            resp = buff;
            break;
        case 5:
            strcpy_P(buff, str_D5);
            resp = buff;
            break;
        case 6:
            strcpy_P(buff, str_D6);
            resp = buff;
            break;
        default:
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_VALUE_ERROR);
            resp += err;
            break;
    }
    strcpy_P(buff, str_COLON);
    resp += buff;

    boolean append_cb = false;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token, e.g. "D2:" or "D2"
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += digitalRead(numpin);
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_ON) == 0) {
        digitalWrite(numpin, HIGH);
        resp += digitalRead(numpin);
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_OFF) == 0) {
        digitalWrite(numpin, LOW);
        resp += digitalRead(numpin);
        append_cb = true;
    } else {                                                 // "D2:3"
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }

    if (append_cb) {
        strcpy_P(buff, str_COLON);
        resp += buff;
        strcpy_P(buff, str_BIN);
        resp += buff;
    }

    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * function: handle_SID
 * appends:  the Station ID (SID) to output display
 * Used for setting the SID or reading current SID
 */
void handle_SID() {
    char buff[10];
    String resp("");
    strcpy_P(buff, (char *)str_SID);
    resp = buff;
    strcpy_P(buff, (char *)str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token, e.g. "D2:" or "D2"
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += SID;
    } else {
        if (strlen(subtokens[1]) < SID_MAX_LENGTH) {
            char * newsid = subtokens[1];        // point to it
            char SIDbuff[SID_MAX_LENGTH];
            for (int i=0; i< SID_MAX_LENGTH - 1 ; i++) {
                SIDbuff[i] = newsid[i];
                if ( SIDbuff[i] == '\0' ) {
                    break;
                }
            }
            strncpy(SID, SIDbuff, SID_MAX_LENGTH);   // copy SIDbuff into SID
            writeSIDToEEPROM();
            resp += SID;
        } else {
            // provided SID was too long
            processing_is_ok = false;
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_ERR_SID_TOO_LONG);
            resp += err;
        }
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
    resp = "";
}


/**
 * function: handle_TID
 * appends:  the Transaction ID (TID) to output display
 * Used for allow host to track message/response pairs.
 */
void handle_TID() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_TID);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else {
        resp += subtokens[1];
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * function: handle_VER
 * appends:  the SNIPE version to output display
 */
void handle_VER() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_VER);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += CURRENT_VERSION;
    } else {
        processing_is_ok=false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * function: handle_DESC
 * appends:  the SNIPE description to output display
 */
void handle_DESC() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_DESC);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += DESCRIPTION;
    } else {
        processing_is_ok=false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

void handle_SLC1() {
    handle_SLC_worker(1);
    return;
}

void handle_SLC2(){
    handle_SLC_worker(2);
    return;
}
void handle_SLC3(){
    handle_SLC_worker(3);
    return;
}

void handle_SLC_worker(uint8_t) {
// TODO
}
void handle_SLM1(){
    handle_SLM_worker(1);
    return;
}
void handle_SLM2(){
    handle_SLM_worker(2);
    return;
}
void handle_SLM3(){
    handle_SLM_worker(3);
    return;
}

void handle_SLM_worker(uint8_t) {
// TODO
}

/**
 * function:  handle_SLA
 * appends:  the state to the output display
 * Used to set the alarm state.
 */
void handle_SLA() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_SLA);
    resp = buff;
    strcpy_P(buff, str_COLON);       // "SLA:"
    resp += buff;

    boolean append_cb = false;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token, e.g. "SLA:" or "SLA"
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += digitalRead(ALARM_PIN);
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_ON) == 0) {
        digitalWrite(ALARM_PIN, HIGH);
        resp += digitalRead(ALARM_PIN);
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_OFF) == 0) {
        digitalWrite(ALARM_PIN, LOW);
        resp += digitalRead(ALARM_PIN);
        append_cb = true;
    } else {                                                 // "D2:3"
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }

    if (append_cb) {
        strcpy_P(buff, str_COLON);
        resp += buff;
        strcpy_P(buff, str_BIN);
        resp += buff;
    }

    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}


/**
 * function: handle_I2A
 * appends:  the I2C Address to output display
 * Used to query or set the I2C target address
 */
void handle_I2A() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_I2A);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += I2C_Address;
    } else  {
        uint16_t addr = atoi(subtokens[1]);    //TODO:  atoi is concat'ing, and it's a uint8 so just rolling over.  Maybe this is ok?
        if ( (addr < 0) || (addr > 255) ) {   // I2C has limited address range
            processing_is_ok = false;
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_OUT_OF_RANGE);
            resp += err;
        } else {
            I2C_Address = addr;
            resp += I2C_Address;
        }
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * function: handle_I2B
 * appends:  the I2C Byte count to output display
 * Used to set or query the number of bytes to read/write
 */
void handle_I2B() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_I2B);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += I2C_Bytes;
    } else  {
        uint8_t b = atoi(subtokens[1]);
        if ( (b < 1) || (b > 17) ) {   // we will only send so many bytes at a time
            processing_is_ok = false;
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_BYTE_SETTING_ERR);
            resp += err;
        } else {
            I2C_Bytes = b;
            resp += I2C_Bytes;
        }
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * function: handle_I2W
 * appends:  the I2W write data to output display
 * Used to set or query the bytes to read/write.  Expects
 * values to be in hexadecimal notation (e.g. "0x0AFF")
 */
void handle_I2W() {
    char buff[10];
    //checkRAMandExitIfLow(5);
    String resp("");
    strcpy_P(buff, str_I2W);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;

    char hexprefix[3];             // should be: '0x'
    hexprefix[0] = subtokens[1][0];
    hexprefix[1] = subtokens[1][1];
    hexprefix[2] = NULL;

    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        char datastr[1 + (2 * I2C_Bytes)];
        //Serial.print(F("# I2C Bytes: "));
        //Serial.print(I2C_Bytes, DEC);
        //Serial.print(F("   Datastr Length: "));
        //Serial.println(1 + (2 * I2C_Bytes), DEC);
        convertByteArrayToHexString(I2C_Data, I2C_Bytes, datastr );
        strcpy_P(buff, str_HEX);
        resp += buff;
        resp += datastr;

    } else if (strcmp_P(hexprefix, str_HEX) == 0) {                      // verify we start with 0x
        // process the write
        char * hexstring = &subtokens[1][2];
        //Serial.print(F("# sub st1: ")); Serial.print(hexstring); Serial.print(F("   length: ")); Serial.println(strlen(subtokens[1])) - 2);
        if ( (strlen(hexstring) == I2C_Bytes * 2)) {                 // st1 contains '0x', so -2 is what we want
            uint8_t bytearraylen = (I2C_Bytes * 2) + 1;                         // +1 NULL
            //char bytechars[bytearraylen];
            //Serial.print(F("# bytearraylen: ")); Serial.println(bytearraylen, DEC);
            //st1.toCharArray(bytechars, arraylen, 2);  // get a standard C-string array
            convertHexStringToByteArray(hexstring, I2C_Data);
            // printBytesAsDec(I2C_Data, I2C_Bytes);

            //-----------
            perform_I2C_write();
            //-----------

            // TODO:  Do the write over I2C now
            resp += subtokens[1];
        } else  {
            processing_is_ok = false;
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_DATA_LENGTH_ERR);
            resp += err;
        }
    } else {
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    //Serial.print(F("RESP: "));
    //Serial.println(resp);
    output_string.concat(resp);
    //checkRAMandExitIfLow(6);           //this is possibly the 'deepest' RAM use point in the program.
}


/**
 * function: handle_I2R
 * appends:  the I2C read data to output display
 * Used to set or query the bytes to read/write.  Expects
 * values to be in hexadecimal notation (e.g. "0x0AFF")
 */
void handle_I2R() {
    char buff[10];

    String resp("");
    strcpy_P(buff, str_I2R);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;

    char hexprefix[3];             // should be: '0x'
    hexprefix[0] = subtokens[1][0];
    hexprefix[1] = subtokens[1][1];
    hexprefix[2] = NULL;

    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        perform_I2C_read();   // this loads the data into I2C_Data
        char datastr[1 + (2 * I2C_Bytes)];
        convertByteArrayToHexString(I2C_Data, I2C_Bytes, datastr );
        strcpy_P(buff, str_HEX);
        resp += buff;
        resp += datastr;
    } else {
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    //Serial.print(F("RESP: "));
    //Serial.println(resp);
    output_string.concat(resp);
    //checkRAMandExitIfLow(6);           //this is possibly the 'deepest' RAM use point in the program.
}




/**
 * function: handle_I2S
 * appends:  the I2C Setting to output display
 * Used to set or query the setting (or 'register') address
 */
void handle_I2S() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_I2S);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += I2C_Register;
    } else  {
        uint16_t reg = atoi(subtokens[1]);  //TODO: Truncates to uint8, so out of range is meaningless
        if ( (reg < 0) || (reg > 255) ) {   // I2C has limited address range
            processing_is_ok = false;
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_OUT_OF_RANGE);
            resp += err;
        } else {
            I2C_Register = reg;
            resp += I2C_Register;
        }
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * function: handle_I2F
 * appends:  the I2C Found devices to output display
 * Used to query for devices on the I2C bus.
 */
void handle_I2F() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_I2F);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        uint8_t address = 1;
        uint8_t error = 0;
        uint8_t num_devices = 0;

        for(address = 1; address <=127; address++) {
            // Use the return value of Wire.endTransmission
            // to see if a device ack'd.
            Wire.beginTransmission(address);
            //wiresend('a');
            //Serial.println(F("begin"));
            error = Wire.endTransmission();
            //I2C_Register=2;
            //I2C_Address=address;
            //I2C_Bytes=1;
            //I2C_Data=uint8_t (['a']);
            //error=perform_I2C_write_error();
            //Serial.println(F("end"));
            if (error == 0) {
                // FOUND one
                //Serial.println(F("#found one"));
                resp += address;
                strcpy_P(buff, str_COMMA);
                resp += buff;
                num_devices++;
            } else {
                // error == 4: Unknown error at this address
                // do nothing;
            }
        }
        if (num_devices == 0 ) {
            char nf[MAX_ERROR_STRING_LENGTH];
            strcpy_P(nf, str_NONE_FOUND);
            resp += nf;
        }
    }  else {
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}






/**
 * function: handle_BLINK
 * appends:  the LED BLINK time to output display
 * Used to blink the Arduino LED for user identification.
 */
void handle_BLINK() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_BLINK);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else  {
        blink_time = atoi(subtokens[1]);
        is_blinking = true;
        blink_start = millis();
        last_blink_change = blink_start;
        resp += blink_time;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}



#pragma mark Setup Helpers
/***************************************************
 ***************************************************
 *   SETUP HELPERS
 ***************************************************
 ***************************************************/

void serialPrintHeaderString() {
    Serial.println();
    Serial.println(F("#####HEADER#####"));
    Serial.println(F("#--------------------------------------------------"));
    Serial.println(F("# SNIPE for Arduino"));
    Serial.println(F("#--------------------------------------------------"));
    Serial.println(F("# Red Byer    github.com/mizraith"));
    Serial.println(F("# VERSION DATE: 2/27/2019"));
    Serial.print(F("# COMPILED ON: "));
    Serial.print(COMPILED_ON.month());
    Serial.print(F("/"));
    Serial.print(COMPILED_ON.day());
    Serial.print(F("/"));
    Serial.print(COMPILED_ON.year());
    Serial.print(F(" @ "));
    Serial.print(COMPILED_ON.hour(), DEC);
    Serial.println(COMPILED_ON.minute(), DEC);
    Serial.println(F("#--------------------------------------------------"));
    Serial.println(F("#"));
}

void printString_P( const char str[]) {
    char c;
    if(!str) return;
    while( (c = pgm_read_byte(str++))) {
        Serial.print(c);
    }
}

void printSerialInputInstructions( void ) {
    char * buff[10];
    Serial.println(F("#"));
    Serial.println(F("#--------------------------------------------------"));
    Serial.println(F("# Serial Control Instructions"    ));
    Serial.println(F("#   Format:  >CMD:VALUE<CR><LF>"    ));
    Serial.println(F("#   Lead characters are:"));
    Serial.println(F("#   > = start   @ = resp  # = comment  ! = error"    ));
    Serial.println(F("#   Commands are:"));
    Serial.print  (F("#   "));
    printString_P(str_A0); printString_P(str_SPACE); printString_P(str_A1); printString_P(str_SPACE);
    printString_P(str_A2); printString_P(str_SPACE); printString_P(str_A3); printString_P(str_SPACE);
    printString_P(str_D2); printString_P(str_SPACE); printString_P(str_D3); printString_P(str_SPACE);
    printString_P(str_D4); printString_P(str_SPACE); printString_P(str_D5); printString_P(str_SPACE);
    printString_P(str_D6);
    Serial.println();
    Serial.print  (F("#   "));
    printString_P(str_SID); printString_P(str_SPACE); printString_P(str_TID); printString_P(str_SPACE);
    printString_P(str_VER); printString_P(str_SPACE);
    printString_P(str_DESC); printString_P(str_SPACE); printString_P(str_BLINK);
    Serial.println();
    Serial.print  (F("#   "));
    printString_P(str_I2A); printString_P(str_SPACE); printString_P(str_I2S); printString_P(str_SPACE);
    printString_P(str_I2B); printString_P(str_SPACE); printString_P(str_I2W); printString_P(str_SPACE);
    printString_P(str_I2R); printString_P(str_SPACE); printString_P(str_I2F);
    Serial.println();
    Serial.println(F("#   EXAMPLES:   >A0:?         >D6:1      >SID:2929"  ));
    Serial.println(F("#   RESPONSES:  @A0:123:ARB   @D6:1:BIN  @SID:2929"  ));
    Serial.println(F("#   Token is   IDENTIFIER:VALUE:UNITS  where UNITS is optional "  ));
    Serial.print  F(("#   Max Input String Length: "));
    Serial.println(MAX_INPUT_LENGTH, DEC);
    Serial.print  F(("#   Max Number of Tokens: "));
    Serial.println(MAX_NUMBER_TOKENS, DEC);
    Serial.print  F(("#   Max SID Length: "));
    Serial.println(SID_MAX_LENGTH, DEC);
    Serial.print  F(("#   Max Output Length: "));
    Serial.println(MAX_OUTPUT_LENGTH, DEC);
    Serial.println(F("#--------------------------------------------------"));
}

void printSerialDataStart() {
    Serial.println(F("#"));
    Serial.print(F("# Current Station ID: "));
    Serial.println(SID);
    Serial.println(F("#"));
    Serial.println("#####DATA#####");
}


#pragma mark EEPROM helpers
/***************************************************
 ***************************************************
 *   EEPROM HELPERS
 ***************************************************
 ***************************************************/
// Virgin AVR EEPROMs start at 0xFF, which can
// make a mess of string checking.
// This method is intended to be called during setup
// to see if an EEPROM blank is required.  This
// method basically loops through the first 10
// addresses in EEPROM and if they all equal 0xFF
// it will return 1 (true).  If not virgin, returns 0 (false).
uint8_t isVirginEEPROM() {
    uint8_t result = 1;
    uint8_t val = 0xFF;
    Serial.print(F("# Setup: Checking EEPROM:"));
    for( int i=0; i < 10; i++) {
        val = EEPROM.read(i);
        if (val != 0xFF) {
            Serial.println(F(" NOT a virgin EEPROM"));
            return 0;
        }
    }
    Serial.println(F(" IS a virgin EEPROM"));
    return 1;
}

// I prefer my EEPROMS init to 0
// pass in numbytes (for atmega328p should be 1024)
void initEEPROM(int numbytes, uint8_t initvalue) {
    for (int i=0; i < numbytes; i++) {
        EEPROM.write(i, initvalue);
    }
}


void readSIDFromEEPROM( ) {
    for (int i=0; i < SID_MAX_LENGTH-1; i++) {
        SID[i] = EEPROM.read(i + SID_EEPROM_START_ADDRESS);
        if ( SID[i] == '\0' )  {
            return;          // reached string end
        }
        if (i == SID_MAX_LENGTH - 2) {     //coming up on end
            SID[SID_MAX_LENGTH-1] = '\0';  //make sure end is terminated
            return;
        }

    }
}

void writeSIDToEEPROM( ) {
    for (int i=0; i < SID_MAX_LENGTH-1; i++) {
        EEPROM.write(i + SID_EEPROM_START_ADDRESS, SID[i]);
        if ( SID[i] == '\0' ) {
            return;
        }
        if (i == SID_MAX_LENGTH - 2) {          // coming up on end
            //make sure to finish with a null
            EEPROM.write( (SID_MAX_LENGTH-1) + SID_EEPROM_START_ADDRESS, '\0' );  // last char must be null
            return;
        }
    }
}










#pragma mark Low Level Helpers
/***************************************************
 ***************************************************
 *   LOW LEVEL HELPERS
 ***************************************************
 ***************************************************/

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
// We need to know numbytes in our array, since we cannot count on
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
    target[t] = NULL;
    return;
}




// get the 4 bit nibble 0x0000xxxx from a character
// representing a hexadecimal number
byte getHexNibbleFromChar(char c) {
    if ((c >= '0') && (c <= '9')) {
        return (byte)(c - '0');
    }
    if ((c >= 'A') && (c <= 'F')){
        return (byte)(c - 'A' + 10);
    }
    if ((c >= 'a') && (c <= 'f')) {
        return (byte)(c - 'a' + 10);
    }
}

// get the character from the 4-bit 0x0000xxxx
// nibble representing a hexadecimal number
char getCharFromHexNibble(uint8_t b) {
    if ((b >= 0) && (b <= 9)) {
        return (char)(b + '0');
    } else {
        return (char)(b - 10 + 'A');
    };
}


void printBytesAsDec(uint8_t *data, uint8_t len) {
    Serial.print(F("# ByteArrayAsDec: "));
    for(uint8_t i=0; i<len; i++) {
        Serial.print(data[i], DEC);
        Serial.print(F("  "));
    }
    Serial.println();
}





#pragma mark I2C Helpers
/***************************************************
 ***************************************************
 *   RAM Helpers
 ***************************************************
 ***************************************************/
// minihelper, thank you Adafruit for this trinket
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


/*
 * function: perform_I2C_write
 * Given all the I2C settings (I2A, I2B, I2S, data) perorm
 * an I2C write.
 */
void perform_I2C_write() {
    Wire.beginTransmission(I2C_Address);
    wiresend(I2C_Register);
    for (uint8_t i=0; i < I2C_Bytes; i++) {
        wiresend(I2C_Data[i]);
    }
    Wire.endTransmission();
}

int perform_I2C_write_error() {
    Wire.beginTransmission(I2C_Address);
    wiresend(I2C_Register);
    for (uint8_t i=0; i < I2C_Bytes; i++) {
        wiresend(I2C_Data[i]);
    }
    return Wire.endTransmission();
}

/*
 * function: perform_I2C_read
 * Given all the I2C settings (I2A, I2B, I2S, data) perform
 * an I2C write.
 */
void perform_I2C_read() {
    Wire.beginTransmission(I2C_Address);
    wiresend(I2C_Register);
    Wire.endTransmission();
    //delay(1);
    Wire.requestFrom(I2C_Address, I2C_Bytes);
    for (uint8_t i=0; i < I2C_Bytes; i++) {
        I2C_Data[i] = wirereceive();
    }
    Wire.endTransmission();
}










#pragma mark RAM Helpers
/***************************************************
 ***************************************************
 *   RAM Helpers
 ***************************************************
 ***************************************************/

void checkRAMandExitIfLow( uint8_t checkpointnum ) {
    int x = freeRam();
    if (x < 128) {
        Serial.print(F("!!! WARNING LOW SRAM !! FREE RAM at checkpoint "));
        Serial.print(checkpointnum, DEC);
        Serial.print(F(" is: "));
        Serial.println(x, DEC);
        Serial.println();
        gotoEndLoop();
    } else {
        Serial.print(F("# FREE RAM, at checkpoint "));
        Serial.print(checkpointnum, DEC);
        Serial.print(F(" is: "));
        Serial.println(x, DEC);
    }
}

void checkRAM() {
    char buff[10];
    String resp("");
    strcpy_P(buff, str_RAM);
    resp = buff;
    strcpy_P(buff, str_COLON);
    resp += buff;
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        int x = freeRam();
        resp += x;
    } else {
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }
    strcpy_P(buff, str_SPACE);
    resp += buff;
    //Serial.print(F("RESP: "));
    //Serial.println(resp);
    output_string.concat(resp);
}



//Endpoint for program (really for debugging)
void gotoEndLoop( void ) {
    Serial.println(F("--->END<---"));
    while (1) {
        //delay(1);
    }
}


/**
 * Extremely useful method for checking AVR RAM status.  I've used it extensively and trust it!
 * see: http://playground.arduino.cc/Code/AvailableMemory
 * and source:  http://www.controllerprojects.com/2011/05/23/determining-sram-usage-on-arduino/
 */
int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

