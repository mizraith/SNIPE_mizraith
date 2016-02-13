# SNIPE_mizraith
**Extensible serial to I2C-and-more tool.  Easily control your Arduino over a comm port.**

_Grammar Version:_   1.0


#### License:
<i>The MIT License (MIT)

Copyright (c) 2016 Red Byer

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
knife instrument.   SNIPE exposes a simple (and easily extended) serial
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



## Serial Protocol:
The serial protocol aims to be readily extended.  It is based on
tokens and subtokens.  Optional elements are kept to a minimum.
For ease of flow control, a start character is used for both
commands and responses.
    
### Messages:
**Messages** take on the basic form of (up to length 96 chars):

         cTOKEN TOKEN TOKEN .... <CR><LF>

Where "c" is the **start character**, e.g, '>' or '!' or '@' or '#'.

**Tokens** are seperated by one space.  This allows a message
to be easily tokenized by the following:

        >>>  message.strip().split(" ")

Where .strip() removes whitespace at the front or end of message that
may have been added.

Note that it possible to send two or more command tokens
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
- **example:**      `A0:254:ARB`
- _Note:  A0 is the recommended standard analog input._
              
##### D1, D2, D3, D4, D5, [D6]    
- **description:**  Digital 6 readback
- **value range:**  0 : 1
- **units:**        BIN
- **example:**      `D6:0:BIN`
- _Note:     D6 is the recommended standard digital output since it is not used for much on the Arduino._
              
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
        
##### [A0], A1, A2, A3
- **description:**     Get the Analog input
- **input argument:**  ?
- **value range:**     0 - 1024
- **units:**           ARB
- **example:**         
  - command:  `>A0:?`
  - response: `@A0:254:ARB`
- _Note: A0 is the recommended standard analog input._
              
##### D2, D3, D4, D5, [D6]    
- **description:**     Set or Get the digital pin value
- **input argument:**  ?    or   0:1
- **value range:**     0 : 1
- **units:**           BIN
- **example:**         
  - command:  `>D6:?`
  - response: `@D6:0:BIN`
  - command:  `>D6:1`
  - response: `@D6:1:BIN` 
- _Note:  D6 is the recommended standard digital output since it is not used for much on the Arduino._
              
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
- **input argument:**  String, less than 8 characters please
- **example:**         
  - command:  `>A0:? TID:1a35`
  - response: `@A0:245 TID:1a35`
              
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
- **input argument:**  0:5000   Number of milliseconds to blink for
- **example:**         
  - command:  `>BLINK:5000`
  - response: `@BLINK:5000`
        
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
debugging.
