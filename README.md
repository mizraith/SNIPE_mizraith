# SNIPE_mizraith
**Extensible serial to I2C-and-more tool.  Easily control your Arduino over a comm port. Now with Andon Stack Light Tower type features.  Control some neopixels (slowly) over serial!**

_Major Release:_ 4.0

_Grammar Version:_   2.0

_Changes since 1.0:  Added TID:?, BLINK:? and ECHO:[0:1] commands._

_Changes in 4.0:  Added Stack light commands_.
    ECHO removed -- 2 line responses suck to parse sometimes.
    TID removed -- never really used.  Makes parsing more complex.

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
        * [DESC](#desc-)
        * [VER](#ver-)
  * [Commands:](#commands)
    * [Pin Control Commands](#pin-control-commands-)
        * [[A0], A1, A2, A3](#a0-a1-a2-a3-1)
        * [D2, D3, D4, D5, D6](#d2-d3-d4-d5-d6-)
    * [Macro Commands](#macro-commands)
        * [ECHO](#echo-1)
        * [SID](#sid--1)
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
- **input argument:** ? or color value  0xRRGGBB. 
- **input values:** 
  - As hex string: '0x123456'  Leading 0x required.
  - As decimal:  '255'    No leading zero.  Max value <= 0xFFFFFF
  - As text: "RED" "red"  case insensitive.  Matching one of the defined colors.
  - If not properly formatted, will return a value error.  Setting to 0x0 will turn the color "black", or off. Case insensitive.
  - If a color is defined, will append that name to the response string
  **example:** 
  - command: `>SLC1:0xFF0000`
  - response:`@SLC1:0xFF0000`   The LED on STack Light 1 is now RED.
  - command: `>SLC1:0x0`
  - response: `@SLC1:0x0`    The LED is now off.
  - command: `>SLC1:RED`    (or `>SLC1:red`)
  - response: `@SLC1:0xFF0000:RED`   Note it returns the hex value
  - command: `>SLC1:255`        Decimal input
  - response: `@SLC1:0x0000FF:BLUE`   Converts to HEX and appends known color value
  - 
- _NOTE:  **SLC1** --> **[D7]**, **SLC2** -->**[D8]**, **SLC3** --> **[D9]**_

#### SLM1, SLM2, SLM3
- **description:**  Stack Light Mode.  Sets the stack light mode for the target.
- **input argument:** ? or integer argument.
- **input values:**
  - 0:  Off (Same as `SLC:0x0` but retains set color.)
  - 1:  On, Steady State
  - 2:  On, Pulsing.  Option subtoken for full-cycle blink rate in ms.
  - 3:  On, Flashing.  Optional subtoken for full-cycle blink rate in ms.
- **cycle rate argument:** <optional>
  - in milliseconds full cycle time.
  - default: 1000
  - MIN: 100   (setting below this -> 100)
  - MAX: 10000  (setting above this -> 5000)
- **example:**
  - command: `>SLM1:1`
  - response:`@SLM1:1`   The mode for #1 is ON, Steady State.
  - command: `>SLM1:3:500`  The mode for #3 is now flashing every 500ms
  - response: `@SLM1:3:500`  This would round up to 100ms pulse rate.
  - command: `>SLM1:3:55`  The mode for #3 is no flashing every 100ms
  - repsonse: `@SLM1:3:100`  Returns the value it rounded up to.
  - command: `>SLM1:0`
  - response: `@SLM1:0`  The light for stack light #1 is off
  - command: `>SLM1:0:1000`   You can even set the cycle time in steady mode (to carry forward)
  - response: `@SLM1:0:1000`
  - command:  `>SLM1:1:1000.8`  You *can* send a float, but everything after the "." is ignored.
  - response: `@SLM1:1:1000`
  - command:  `>SLM1:1:abcd`   Alpha's won't get parsed as values
  - response:  `@SLM1:1:VALUE_ERROR`   The mode is accepted (so "@", not "!" but VALUE_ERROR for cycle subtoken)
  
#### SLP1, SLP2, SLP3
- **description** Stack Light Percentage   Set how many of the lights are on
- **input argument** ? or int percentage of pixels on.
- **input values** 0:100  0=all off  100=all on.  50=first half closest to controller are illuminated.
- **example:**
  - command: `>SLP1:100`
  - response: `@SLP1:100`   All lights are on
  - command: `>SLP1:25`
  - response: `@SLP1:25`    25% of the lights are on, closest to controller
  - command: `>SLP1:101`    Unlike cycle time, we currently do not round percentages
  - response: `!SLP1:VALUE_ERROR
  - command:  `>SLP1:0x28`  Hex is accepted
  - response: `@SLP1:40`
  - command: `>SLP1:50.3`    But floats are NOT
  - response: `SLP1:VALUE_ERROR`
  - command: `>SLP1:?`         standard query syntax
  - response: `@SLP1:40`

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
  - command:  `>I2A:110 I2S:23 I2B:2 I2W:0xA0F3`
  - response: `@I2A:110 I2S:23 I2B:2 I2W:0xA0F3`
  - command:  `>I2W:0xA1F4`    (resend to same chip/addr)
  - response: `@I2W:0xA1F4`
  - command:  `>I2W:0xA2F3FF`  (resend..too many bytes`
  - response: `!I2W:BYTE_SETTING_ERR`
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
  - command:  `>I2A:110 I2S:23 I2B:2 I2R:?`
  - response: `@I2A:110 I2S:23 I2B:2 I2R:0xA0F3`
  - command:  `>I2R:?`      (re-read from same chip/addr)
  - response: `@I2R:0xA0F3`        
  - command:  `>I2A:1 I2R:?`
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
            
            command:  >I2A:110 I2S:32 I2B:1 I2W:0xA2
            response: @I2A:110 I2S:32 I2B:1 I2W:0xA2
        
And you can alter the order as well because I2C Writes
are processed near last and I2C Reads are processed dead last.  

The following would first set the address, setting, bytes, the write
the data, then read the data back.  Note the return string is
formatted in order of operation.

            command:  >I2W:0xA3 I2R:? I2A:110 I2S:32 I2B:1
            response: @I2A:110 I2S:32 I2B:1 I2W:0xA3 I2R:0xA3


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
