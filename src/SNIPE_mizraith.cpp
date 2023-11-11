/* **********************************************************************************************
ProjectName:  SNIPE  ( Serial 'n I2C Processing Equipment )
Author:       Red Byer    
Date:         6/28/2023
 ---------------------------------------------------------------------------------------------

# SNIPE_mizraith
**Extensible serial to I2C-and-more tool.  Easily control your Arduino over a comm port. Now with Andon Stack Light Tower type features.  Control some neopixels (slowly) over serial!**

_Major Release:_ 4.0 rc1

_Grammar Version:_   2.0

_Changes since 1.0:  Added TID:?, BLINK:? and ECHO:[0:1] commands._

_Changes in 4.0:  Added Stack light commands_.
                    ECHO removed -- 2 line responses suck to parse sometimes.
                    TID removed -- never really used.  Makes parsing more complex.

## README
 **See the README.md file for extensive set of grammar definition, explanation and examples.**

##### CHANGELOG:
- 12/22/2015   Original base code written
- 12/28/2015   Initial release of limited version, grammar 0.1
- 1/27/2016    Grammar improvements, removed 'get' command, spec'd I2C
- 2/12/2016    Fully implemented with a python test suite and nearly 100% coverage.
- 6/11/2016    Updated readme with dependencies section.
- 5/2019       Stability Edits
- 6/2023       UPDATE 4.0  includes StackLight commands for Neopixel RGB LED control.
- 8/2023       4.0 still in progress and debugging.

*************************************************************************** */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#pragma mark INCLUDES
#include <limits.h>
//#include <avr/wdt.h>    // for our reset function
#include <Arduino.h>    // we make good use of String() class
#include <avr/pgmspace.h>
#include <EEPROM.h>
// Uses SCL and SDA pins
#include <Wire.h>
// DOWNLOAD on github.com/mizraith  included for the datetime class.  Wire.h makes the RTCLib happy.
#include"mizraith_DateTime.h"   

// PROJECT LIBRARIES
#include "SNIPE_EEPROM.h"
#include "SNIPE_StackLight.h"
#include "SNIPE_Strings.h"
#include "SNIPE_GeneralUtilities.h"
#include "SNIPE_Conversions.h"
#include "SNIPE_Color.h"


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
//    The Adafruit WS2801 library supports WS2801's.  us LED warehouse sells these.
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
// With platformio.ini you can also pass in the USE_NEOPIXEL_LEDS flag
//#define USE_NEOPIXEL_LEDS
// WS2801 is what we will use in production, the underwater larger pixels.
//define USE_WS2801_LEDS
//now let the defines do their work
//#ifdef USE_NEOPIXEL_LEDS
//#include "/Users/red/Documents/Computer_Science/ARDUINO_DEVELOPMENT/libraries/Adafruit_Neopixel/Adafruit_NeoPixel.h"
    #include "Adafruit_NeoPixel.h"
//#endif  //USE_NEOPIXEL_LEDS

//#ifdef USE_WS2801_LEDS
//    #include "Adafruit_WS2801.h"
//#endif


#pragma mark FORWARD DECLARATIONS
// ********************************************************************************************
//extern void setup();
//extern void loop();
// SETUP
void blinky_worker();
void beepy_worker();
void analog_read_worker();
// SERIAL INTERRUPT FUNCTION
void serialEvent();
// INPUT STRING PROCESSING
void handleInputString();
void handleToken(char *);
void printSubTokenArray();
// SUBTOKEN AND RESPONSE HELPER HANDLERS
void copy_subtoken0colon_into(String &);
bool resp_err_VALUE_MISSING(String &);
void resp_2_output_string(String &);
void resp_err_QUERY_REQUIRED(String &);
// COMMAND HANDLING
void handle_A_worker(uint8_t);
void handle_D_worker(uint8_t);
void handle_SID();
void handle_VER();
void handle_DESC();
void stacklight_startup_sequence();
void handle_SLP_worker(uint8_t);
bool try_handle_stackpercent_query(uint8_t, String &);
bool try_handle_stackpercent_numeric(uint8_t, String &);
void handle_SLC_worker(uint8_t);
bool try_handle_stacklight_query(uint8_t, String &);
bool try_handle_stacklight_numeric(uint8_t, String &);
bool try_handle_stacklight_colorname(uint8_t, String &);
void handle_SLM_worker(uint8_t);
void handle_SLX_worker(uint8_t);
bool try_handle_stackmode_query(uint8_t, String &);
bool try_handle_stacknumled_query(uint8_t, String &);
bool try_handle_stackmode_numeric(uint8_t, String &);
bool try_handle_stacknumled_numeric(uint8_t, String &);
void handle_SLINFO_worker();
void handle_I2A();
void handle_I2B();
void handle_I2W();
void handle_I2R();
void handle_I2S();
void handle_I2F();
void handle_BLINK();
void handle_BEEP();
void handle_HELP();
void handle_REBOOT();

// SETUP HELPERS
void serialPrintHeaderString();
void printSerialInputInstructions();
void printSerialDataStart();
// EEPROM HELPERS
void readSIDFromEEPROM();
void writeSIDToEEPROM();
// LOW LEVEL HELPERS
void prioritize_serial(uint8_t ref);
void printBytesAsDec(uint8_t*, uint8_t);
// I2C HELPERS
void perform_I2C_write();
int perform_I2C_write_error();
void perform_I2C_read();
// Local RAM HELPER
void checkRAM();

#pragma mark Application Globals
const DateTime COMPILED_ON = DateTime(__DATE__, __TIME__);
#define SNIPE_VERSION 4
const String CURRENT_VERSION = "040";
const String DESCRIPTION = "SNIPE_for_Arduino";



#pragma mark Pinouts
// PIN OUTS
#define ANALOG_INPUT   A0
#define LED_PIN        13
#define SL1_PIN        7    // 7 <<< SHOULD BE 7.  Using 5 for debug only based on other design.
#define SL2_PIN        8
#define SL3_PIN        9
#define RELAY_PIN      6
#define BEEP_PIN       6
//#define NUMPIXELS      8   // number of pixels per stacklight.  Nominal 24.

#pragma mark Timing Globals
// baud  115200 / 57600 / 38400 / 19200  /  9600was 57600  but this may be too fast for the nano's interrupts
#define kBAUD_RATE 115200
#define kSERIAL_TIMEOUT_ms 50
// YOUR serialEvent wait limit below is highly dependent on baud rate.  Too short and neopixel.show() could clobber
// serial input.  Too long and you become non-responsive.  Seems correct to roughly your 64char input time.
// At 57600, 5760 chars/sec.  1 char every 0.173ms.   64chars input = 11ms. But string copies
// At 115200, 11520 chars/sec, 1 car every 0.0868ms   64chars input = 5.5ms
const uint8_t kSerialWaitLimit_ms = 5;  //  roughly your 64char input time.
// min and max full cycle time for our flash or pulse modes.
#define kMIN_CYCLE_TIME 100
#define kMAX_CYCLE_TIME 10000


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
//char input_string[MAX_INPUT_LENGTH];       // copy of input_buffer for processing
//char output_string[MAX_OUTPUT_LENGTH];   // buffer for building our output string
char * subtokens[MAX_NUMBER_TOKENS];       // Yes, an array of char* pointers.  We should never have 10 subtokens!
String output_string = "";                 // might as well use the helper libraries supplied by arduino
//char transaction_ID_string[9];           // Transaction ID, max 8 chars

// Parameter 1 = number of pixels in the strip.
// Parameter 2 = SLx_PIN ....Digital pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip1)
//#ifdef USE_NEOPIXEL_LEDS
//Adafruit_NeoPixel SL1_strip = Adafruit_NeoPixel(10, SL1_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel SL2_strip = Adafruit_NeoPixel(8, SL2_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel SL3_strip = Adafruit_NeoPixel(8, SL3_PIN, NEO_GRB + NEO_KHZ800);
//# endif //USE_NEOPIXEL_LEDS


// STACK LIGHT CONTROLLER supporting variables
#define kNUM_STACKLIGHTS 3
SNIPE_StackLight stack_lights[kNUM_STACKLIGHTS] =  {
        SNIPE_StackLight(1, SL1_PIN, 16, NEO_GRB + NEO_KHZ800),
        SNIPE_StackLight(2, SL2_PIN, 60, NEO_GRB + NEO_KHZ800),
        SNIPE_StackLight(3, SL3_PIN, 2, NEO_GRB + NEO_KHZ800),
};

//unsigned long SL_loop_time = 0;
unsigned long SL_next_heartbeat = 25;        //  Timer for our next refresh
unsigned long AIN_next_heartbeat = 25;       //  Timer for analog input reads
uint8_t analog_to_read = 0;
// THIS NEXT impacts serial performance.  Too fast an update speed and too likely to clobber incoming messages.
// @ 25ms  30ms transmissions delay result in 0% failure.   However a 20ms transmission delay yields 4-6% failures.
// @ 50ms  30ms transmissions resulted in 3% failure rate.  Counter-intuitive
// @ 10ms  20ms trasnmissions resulted in 1% failure rate,  30ms resulted in 0%
// @ 1ms   1ms transmission delay = 0% failure......
#define kSL_HEARTBEAT_INTVL_ms 5           // set this low low low. less clobber, less wait for serial

#pragma mark I2C Variables
// Variables for I2C
uint8_t I2C_Address = 0;
uint8_t I2C_Register = 0;
uint8_t I2C_Bytes    = 0;
uint8_t I2C_Data[32];       // set large for init...

#pragma mark Analog Variables
#define kNUM_ANALOG_PINS 4
// Analog Input Variables, conveniently zero indexed
//                       A0      A1     A2     A3
uint16_t A_values[kNUM_ANALOG_PINS] = {0, 0, 0, 0};

#pragma mark Message Processing Variables
// Message Processing Variables
boolean processing_is_ok = true;

#pragma mark State Variables
// Serial Interrupt Variable
volatile boolean input_string_ready = false;

#pragma mark Blink Variables
uint32_t blink_time = 0;
unsigned long blink_start = 0;
unsigned long last_blink_change = 0;
boolean is_blinking = false;
const int blink_toggle_time_ms = 250;

#pragma mark Beep Variables
boolean beep_enabled = false;


#pragma mark Setup & Main
/***************************************************
 ***************************************************
 *   SETUP 
 ***************************************************
 ***************************************************/

void setup() {                 // AT 9600 we don't see any missed serial chars.  Otherwise we'll drop 1 in 25 messages.
    Serial.begin(kBAUD_RATE);
    Serial.setTimeout(kSERIAL_TIMEOUT_ms);
    Wire.begin();
    bool is_virgin_eeprom;

    serialPrintHeaderString();   // print this early to send out ######HEADER######

    is_virgin_eeprom = isVirginEEPROM();

    if (is_virgin_eeprom) {
        Serial.println(F("# __Setup: Init'ing virgin EEPROM."));
        initEEPROM(1024, 0x00);
        Serial.print("# __Setup: Will init SID to: " );
        Serial.println(SID);
        writeSIDToEEPROM();      // write out the default SID
        //TODO:  init_default_settings
    }
    readSIDFromEEPROM();

    struct user_settings * SETTINGS = new struct user_settings;
    loadSettingsFromEEPROM(SETTINGS);  // handles eeprom defaults

    pinMode(ANALOG_INPUT, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(2,  OUTPUT);
    pinMode(3,  OUTPUT);
    pinMode(4,  OUTPUT);
    pinMode(5,  OUTPUT);
    //pinMode(6,  OUTPUT);
    pinMode(BEEP_PIN,OUTPUT);
    pinMode(SL1_PIN, OUTPUT);
    pinMode(SL2_PIN, OUTPUT);
    pinMode(SL3_PIN,OUTPUT);
    pinMode(LED_PIN, OUTPUT);


    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    //digitalWrite(6, LOW);
    digitalWrite(BEEP_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

//    digitalWrite(BEEP_PIN, HIGH);
//    delay(3000);
//    digitalWrite(BEEP_PIN, LOW);

    for (uint8_t lightnum=0; lightnum < kNUM_STACKLIGHTS; lightnum++) {
        stack_lights[lightnum].setup_strip();
    }
//    Serial.println("+++++++++++++++++++++++++++++");
//    Serial.print("WILL LOAD NUMPIXELS into StackLight: ");Serial.println(SETTINGS->sl1_numpixels) ;
//    Serial.println("+++++++++++++++++++++++++++++++");
//   // stack_lights[0] = SNIPE_StackLight(1, SL1_PIN, SETTINGS->sl1_numpixels, NEO_GRB + NEO_KHZ800);


    // Developer note: This next line is key to prevent our output_string (String class)
    // from growing in size over time and fragmenting the heap.  By calling this now
    // we end up saving about 50-60bytes of heap fragmentation.
    output_string.reserve(MAX_OUTPUT_LENGTH);

    checkRAMandExitIfLow(0);

    printSerialInputInstructions();
    // PRINT OUT STACKLIGHT INFO -- should comment out
    handle_SLINFO_worker();

    stacklight_startup_sequence();
    printSerialDataStart();
    SL_next_heartbeat = millis();    // Set to now


}


/***************************************************
 ***************************************************
 *   MAIN LOOP -- Arduino style
 ***************************************************
 ***************************************************/
void loop() {
    // stay responsive on input strings
    if (input_string_ready) {
        handleInputString();
    }

    // PUT slow, low priority or infrequent things behind a heartbeat check
    if (millis() > SL_next_heartbeat) {
        SL_next_heartbeat = millis() + kSL_HEARTBEAT_INTVL_ms;

        blinky_worker();
        beepy_worker();

        for (uint8_t lightnum = 0; lightnum < kNUM_STACKLIGHTS; lightnum++) {
            stack_lights[lightnum].update();
        }
    }
    // We are only going to check analog about once every 100ms
    if (millis() > AIN_next_heartbeat) {
        AIN_next_heartbeat = millis() + 25;
        // analogRead is slow...only do it every so often
        analog_read_worker();
    }
    //delay(15);   // really slows things up, unnecessary
}


void blinky_worker() {
    if (!is_blinking) {
        return;
    }
    unsigned long current_time = millis();
    if (current_time - blink_start > blink_time) {
        digitalWrite(LED_PIN, LOW);
        is_blinking = false;
    } else if (current_time - last_blink_change > blink_toggle_time_ms) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        last_blink_change = millis();
    }
}

void beepy_worker() {
    uint8_t is_beep;
    is_beep = (bool) digitalRead(BEEP_PIN);

    // ---------- beep is off        easiest to handle off condition and exit
    if (!beep_enabled) {
        if (is_beep) {
            digitalWrite(BEEP_PIN, LOW);
        }
        return;
    }

    // ---------- beep is enabled
    // handle steady state modes
    if ((stack_lights[0].get_mode() != MODE_PULSE) and (stack_lights[0].get_mode() != MODE_FLASH)) {
        if (!is_beep) {
            // we need to switch on
            //Serial.print("Stuck in STEADY:");
            digitalWrite(BEEP_PIN, HIGH);
        }
        return;
    }
    // handle pulse modes -- sync with light state...start with a double-check on mode
    if ((stack_lights[0].get_mode() == MODE_PULSE) or (stack_lights[0].get_mode() == MODE_FLASH)) {
        if (stack_lights[0].get_flash_on()) {
            if (!is_beep) {
                // we need to switch on
                //Serial.println("flash on");
                digitalWrite(BEEP_PIN, HIGH);
            }
            // otherwise, it's already on
            return;
        } else {
            if (is_beep) {
                // we need to switch off
                //Serial.println("flash off");
                digitalWrite(BEEP_PIN, LOW);
            }
            //otherwise, it's already off
            return;
        }
    }
}



void analog_read_worker() {
    switch (analog_to_read) {
        case 0:
            A_values[0] = analogRead(A0);
            break;
        case 1:
            A_values[1] = analogRead(A1);
            break;
        case 2:
            A_values[2] = analogRead(A2);
            break;
        case 3:
            A_values[3] = analogRead(A3);
            break;
        default:
            if (analog_to_read > 3) {
                analog_to_read = 0;
            }
    }
}



#pragma mark Serial EVENT Function
/***************************************************
 ***************************************************
//  SERIAL INPUT FUNCTION  (Arduino Library..this is NOT an interrupt)
//  serialEvent() is called at the end of every loop........
//
//     ACTS IMMEDIATELY ONE CHAR AT A TIME TO
//     BUILD UP THE INPUT STRING
//     UNTIL IT SEES A CARRIAGE RETURN OR LINE FEED
//
// Timing notes, at 57600bps, there are 5760 chars per second (10bits per char)
//   That's 1 char every 0.174ms
//   Our long message (>SLCx:YELLOW) is 12 chars or 2ms
//   A much longer 24char message is 4ms and our full length 64 char input length is 11.2ms
//
// I've noted (and confirmed) that Neopixel.show() turns off interrupts, which drops serial comm.
//  At 57600 I will see 15% of messages drop between 1-6 character.  Worst case (6chars) is a 1ms neopixel interruption.
//  At 9600 I'm not seeing any drops (each character takes 1ms)...or at least drops are significantly more rare.
//
// THEREFORE - the recommended strategy will be to:isolate the .show() command and DO NOT CALL it if we a
//   1)  Recognize the start char and set our "accumulating_serial_string" flag
//   2)  centralize .show() and put a hold going into it if "accumulating_serial_string" is true. Hold max of baudrate * 64bits
//           --> This function is called  prioritize_serial();
 //   3)  simplify serialEvent and do the processing outside of it.

 ***************************************************
 ***************************************************/
volatile char c;
String serial_string = String("");
volatile uint8_t cindex = 0;
//const char NONE = -1;
volatile bool accumulating_serial_string = false;

// New New New Version -- accumulate the entire string at once
void serialEvent() {
    while (Serial.available() > 0) {
        c = Serial.read();
        if (c == char_CMD) {
            accumulating_serial_string = true;
            serial_string = String("");
//            noInterrupts();     # Serial.readsStringUntil **requires** interrupts. DO NOT DISABLE THEM
            serial_string = Serial.readStringUntil('\n');   // '\n' TODO  try replacing terminator
            serial_string.trim();
//            interrupts();
            input_string_ready = true;
            accumulating_serial_string = false;
            return;
        } else if (!isspace(c)) {    // ------ AWAITING START CHARACTER
                char buff[MAX_ERROR_STRING_LENGTH];
                strcpy_P(buff, str_INVALID);
                accumulating_serial_string = false;
                input_string_ready = false;
                Serial.print(buff);Serial.println(c);
        }
    }
}


// New version -- do less, do it faster and don't get interrupted!
//void serialEvent() {
//    noInterrupts();
//    while (Serial.available() > 0) {
//        c = Serial.read();
//
//        if (accumulating_serial_string) {
//            if (c == char_CMD) {
//                // We will strip out (ignore) additional command chars:  ">SLM1:1 >SLM2:1"  -->  "SLM1:1  SLM2:2"
//                // as this is probably what the user intended in the first place.
//                // THEREFORE, DO NOTHING
//            } else if ((c == char_CR) ||
//                       (c == char_LF) ||
//                       (cindex > (MAX_INPUT_LENGTH - 2))) {
//                // end of string condition handling
//                input_buffer[cindex] = '\0';   // null terminator
//                input_string_ready = true;
//                accumulating_serial_string = false;
//                cindex = 0;
//                //            if (cindex > (MAX_INPUT_LENGTH - 2) ){
//                //                Serial.println(F("!INPUT_IS_TOO_LONG!"));
//                //            }
//                interrupts();
//                return;         // end of command...return to handle and come back later
//            } else {
//                input_buffer[cindex] = c;
//                cindex++;
//            }
//
//        } else {  // NOT ACCUMULATING
//            if (c == char_CMD) {
//                accumulating_serial_string = true;
//                cindex = 0;    // reset to 0 position
//                //strcpy(input_buffer, "");  //blank it to accumulate
//            } else if ((c == char_CR) ||
//                       (c == char_LF)) {
//                accumulating_serial_string = false;
//                cindex = 0;
//                //return;  // spurious newlines during non accumulation
//            } else {  // not a command or newline
//                // we're not accumulating and are getting characters...yuck!
//                Serial.print(F("!Invalid_input:"));
//                Serial.print(c);
//                Serial.println(F(":Commands_start_with_>"));
//            }
//        }
//        //c = NONE;       //reset
//    }
//    interrupts();
//}


#pragma mark Input String Processing
/***************************************************
 ***************************************************
//  MAIN INPUT HANDLING FUNCTIONS
 ***************************************************
 ***************************************************/
void handleInputString() {
    //Note that the ">" has already been removed by this point.
    //checkRAMandExitIfLow(1);
    //long time_start = millis();
    processing_is_ok = true;  // leave as true....only set false if we encounter an error

    //Serial.print(F("# RECEIVED_INPUT->"));
    //Serial.println(input_string);

    char* token;
    char* tempstring;
    char* tempstringptr;

    output_string = "";                 // clear it

    //char cpystring[MAX_INPUT_LENGTH];
    //strcpy(cpystring, input_buffer);

    //tempstring = strdup(input_buffer);

    // NEW NEW NEW code
    //serial_string.trim();   // remove whitespace -- done by serialEvent()
    //Serial.print("#serial_string---->");Serial.println(serial_string);
    tempstring = strdup(serial_string.c_str());
    //Serial.print("#tempstring------->");Serial.println(tempstring);

    tempstringptr = tempstring;    //store copy for later release

    if (tempstring != NULL) {
        char space[3];
        strcpy_P(space, str_SPACE);
        // *** SPLIT INTO MAIN TOKENS ****
        while ((token = strsep(&tempstringptr, space)) != NULL)  {
            //if we get 2 spaces together we get a 0-length token and we should ignore it
            if ( ( token != NULL ) && (strlen(token) != 0 )  ) {
                Serial.print("#token------------>");Serial.println(token);
                handleToken(token);
            }
        }  // end main token handling
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
    free(tempstringptr);   // GETTING RID OF THIS LINE SIGNIFICANTLY INCREASES FAILURES   !?
    free(tempstring);

    //long time_end = millis();
    //long totaltime = time_end - time_start;
    //Serial.print(F("# Total execution (ms): "));
    //Serial.println(totaltime, DEC);
    //checkRAMandExitIfLow(11);
    input_string_ready = false;  // now that we have a copy, we can prep to accum more.
}


void handleToken(char* ctoken) {
    //Note that the ">" has already been removed by this point, so subtokens[0] contains the full command
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
    // SPLIT INTO SUBTOKENS AND STORE IN OUR GLOBAL ARRAY, so we don't have to do the work again
    uint8_t i = 0;
    while ((subtoken = strsep(&tokentofree, colon)) != NULL) {
        if (subtoken != NULL) {    // a 0-length subtoken _is_ acceptable...maybe there is no value.
            //Serial.print("#subtoken--------->");Serial.println(subtoken);
            subtokens[i] = subtoken;
        }
        i++;
    }  // end subtoken split
    subtokens[i] = NULL;  // End list with NULL flag
    // Debug
    Serial.print("#subtokens0------->");Serial.println(subtokens[0]);
    Serial.print("#subtokens1------->");Serial.println(subtokens[1]);


    //checkRAMandExitIfLow(22);

    // make cmd into uppercase so that our commands are case insensitive
    i = 0;
    while (subtokens[0][i])
    {
        c = subtokens[0][i];
        subtokens[0][i] = toupper(c);
        i++;
    }
    // MATCH OUR TOKEN WITH OUR HANDLE-ABLE STRINGS
    if        (strcmp_P(subtokens[0], str_A0) == 0) {
        handle_A_worker(0);
    } else if (strcmp_P(subtokens[0], str_A1) == 0)  {
        handle_A_worker(1);
    } else if (strcmp_P(subtokens[0], str_A2) == 0)  {
        handle_A_worker(2);
    } else if (strcmp_P(subtokens[0], str_A3) == 0)  {
        handle_A_worker(3);
    } else if (strcmp_P(subtokens[0], str_D2) == 0)  {
        handle_D_worker(2);
    } else if (strcmp_P(subtokens[0], str_D3) == 0)  {
        handle_D_worker(3);
    } else if (strcmp_P(subtokens[0], str_D4) == 0)  {
        handle_D_worker(4);
    } else if (strcmp_P(subtokens[0], str_D5) == 0) {
        handle_D_worker(5 );
    } else if (strcmp_P(subtokens[0], str_D6) == 0) {
        handle_D_worker(6);
    } else if (strcmp_P(subtokens[0], str_SLP1) == 0) {
        handle_SLP_worker(1);
    } else if (strcmp_P(subtokens[0], str_SLP2) == 0) {
        handle_SLP_worker(2);
    } else if (strcmp_P(subtokens[0], str_SLP3) == 0) {
        handle_SLP_worker(3);
    } else if (strcmp_P(subtokens[0], str_SLC1) == 0) {
        handle_SLC_worker(1);
    } else if (strcmp_P(subtokens[0], str_SLC2) == 0) {
        handle_SLC_worker(2);
    } else if (strcmp_P(subtokens[0], str_SLC3) == 0) {
        handle_SLC_worker(3);
    } else if (strcmp_P(subtokens[0], str_SLM1) == 0) {
        handle_SLM_worker(1);
    } else if (strcmp_P(subtokens[0], str_SLM2) == 0) {
        handle_SLM_worker(2);
    } else if (strcmp_P(subtokens[0], str_SLM3) == 0) {
        handle_SLM_worker(3);
    } else if (strcmp_P(subtokens[0], str_SLINFO) == 0) {
        handle_SLINFO_worker();
    } else if (strcmp_P(subtokens[0], str_SID) == 0) {     // could use (0 == strcmp(subtokens[0], "SID")) (strcmp_P(subtokens[0], PSTR("SID")) == 0 ) (cmd.equalsIgnoreCase(str_SID))
        handle_SID();
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
    } else if (strcmp_P(subtokens[0], str_BEEP) == 0) {
        handle_BEEP();
    } else if (strcmp_P(subtokens[0], str_RAM) == 0) {
        checkRAM();
    } else if (strcmp_P(subtokens[0], str_REBOOT) == 0) {
        handle_REBOOT();
    } else {
        processing_is_ok = false;
        String resp("");
        char err[MAX_ERROR_STRING_LENGTH];
        resp += subtokens[0];
        strcpy_P(err, str_COLON);
        resp += err;
        strcpy_P(err, str_UNKNOWN_CMD_ERR);
        resp += err;
        strcpy_P(err, str_SPACE);
        resp += err;
        output_string.concat(resp);
        // add bad response to output_string
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
    free(tokentofree);    // REMOVING THIS LINE INCREASES FAILURES SIGNIFICANTLY
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

/**
 * Simple..add the first subtoken back onto the response string
 * @param resp
 */
void copy_subtoken0colon_into(String & resp) {
    char buff[2];
    resp += subtokens[0];    // copy over "A0"
    strcpy_P(buff, str_COLON);  //   resp --> "Ax:"
    resp += buff;
//    resp += ":";
}

/**
 * Simple -- check if the second subtoken is null or 0.
 * @param resp  The string to append error messages to
 * @return true if value is null or missing.  false if there is a value
 */
bool resp_err_VALUE_MISSING(String & resp) {
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
        return true;
    } else {
        return false;
    }
}

/**
 * Copy the response into our output string for transmission back over serial.
 * Every function does this at the end.
 * @param resp The response we have built up so far.
 */
void resp_2_output_string(String & resp) {
    char buff[3];
    strcpy_P(buff, str_SPACE);   // forgot why we do this...there's some reason
    resp += buff;
    output_string.concat(resp);
}

/**
 * Add ERROR required to our response
 * @param resp
 */
void resp_err_QUERY_REQUIRED(String & resp) {
    char err[MAX_ERROR_STRING_LENGTH];
    strcpy_P(err , str_Q_REQUIRED);
    resp +=  err;
}


#pragma mark COMMAND HANDLER EXPLANATION
/**
 * Command handlers must all share the same philosophy.
 * READS FROM:  the subtokens array  (pointers to char* strings)
 *               e.g.  subtokens[0] = "SID"   subtokens[1] = "1234" subtokens[2]=NULL
 * IMPACTS:  processing_is_ok   sets to false if something goes wrong
 * APPENDS:  its results to output_string
 */
/***************************************************
 ***************************************************
//  COMMAND HANDLING FUNCTIONS
 ***************************************************
 ***************************************************/
#pragma mark Command Handling

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

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        val = A_values[numA];
        resp += String(val);                  // Arduino String allows concat of an int, but must be on its own line
        strcpy_P(buff, str_COLON);
        resp += buff;
        strcpy_P(buff, str_ARB);
        resp += buff;
        processing_is_ok=true;
    } else {
        processing_is_ok=false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err , str_Q_REQUIRED);
        resp +=  err;
    }
    resp_2_output_string(resp);
}


/**
 * function: handle_D_worker
 * numpin:   Arduino digital input
 * appends:  digital pin status to output display
 */
void handle_D_worker(uint8_t numpin) {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    boolean append_cb = false;
    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

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

    resp_2_output_string(resp);
}

/**
 * function: handle_SID
 * appends:  the Station ID (SID) to output display
 * Used for setting the SID or reading current SID
 */
void handle_SID() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

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

    resp_2_output_string(resp);
}


/**
 * function: handle_VER
 * appends:  the SNIPE version to output display
 */
void handle_VER() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += CURRENT_VERSION;
    } else {
        processing_is_ok=false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }

    resp_2_output_string(resp);
}

/**
 * function: handle_DESC
 * appends:  the SNIPE description to output display
 */
void handle_DESC() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += DESCRIPTION;
    } else {
        processing_is_ok=false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_Q_REQUIRED);
        resp += err;
    }
    resp_2_output_string(resp);
}

void stacklight_startup_sequence() {
    Serial.println(F("# Doing Stack Light Startup Sequence"));
    const uint8_t numwashclrs = 5;
    uint32_t wash_colors[numwashclrs] = {cRED.value, cYELLOW.value, cGREEN.value, cBLUE.value, cBLACK.value};
    uint32_t wash_color;
    uint8_t n = 0;
    uint8_t max_numpixels = 0;
    for (uint8_t lightnum=0; lightnum < kNUM_STACKLIGHTS; lightnum++) {
        max_numpixels = max(max_numpixels, stack_lights[lightnum].numpixels );
    }
    for(uint8_t x=0; x < numwashclrs; x++) {
        wash_color = wash_colors[x];
        for (uint8_t i = 0; i < max_numpixels; i++) {
            for (uint8_t lightnum=0; lightnum < kNUM_STACKLIGHTS; lightnum++) {
                //Serial.print("i: ");Serial.print(i);
                n = min(i, stack_lights[lightnum].numpixels - 1);
                //Serial.print("  numpix: ");Serial.println(stack_lights[lightnum].numpixels);
                stack_lights[lightnum].strip->setPixelColor(n, wash_color);

                prioritize_serial(lightnum + 1);
                stack_lights[lightnum].strip->show();
                delay(3);
            }
        }
        delay(50);
    }
}


/**
 * We've learned that we need to run this BEFORE calling show() for neopixels.
 * Delay until our serial is done doing its thing.
 * ref:  number of the strip that is waiting
 */
void prioritize_serial(uint8_t ref) {
    if (Serial.available()) {
        Serial.print("#>>>>>>>>>>>>>>>>>>>> Potential Collision on ");Serial.println(ref, DEC);
        delay(kSerialWaitLimit_ms);  // all it takes to let 24 bits through at 115200 baud
        return;
    }
    //Serial.print(ref, DEC);Serial.print("   entry:");Serial.print(entry, DEC);Serial.print(" exit:");Serial.print(millis(), DEC);Serial.println();
}



/**
 * function: handle_SLP_worker
 * sl_num:  which stack light are you controlling?  1, 2, or 3
 * Handle Stack Light Percentage worker
 * Expects values to be an int between 0 and 100 notation (e.g. "0x0AFF") or a color in all caps ("RED")
 */
void handle_SLP_worker(uint8_t sl_num) {
    bool handled = false;
    char buff[8];
    char err[MAX_ERROR_STRING_LENGTH];
    String resp("");
    processing_is_ok = true;

    copy_subtoken0colon_into(resp);

    // STEP 2:  DO WE HAVE SUBTOKENS
    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;
        handled = true;

        // STEP 2A:   WE GOT SUBTOKENS
    } else if ((sl_num >= 1) && (sl_num <= kNUM_STACKLIGHTS + 1)) {  // this is a triple check actually

        handled = try_handle_stackpercent_query(sl_num, resp);

        if (!handled) {
            handled = try_handle_stackpercent_numeric(sl_num, resp);
        }
    }    // ...NOT SURE HOW TO PROCESS
    if (!handled) {
        processing_is_ok = false;
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }
    resp_2_output_string(resp);

}

bool try_handle_stackpercent_query(uint8_t sl_num, String & resp) {
    if (strcmp_P(subtokens[1], str_QUERY) != 0) {
        return false;
    }
    //  Got  SLP1:?    return   SLP1:99
    resp += stack_lights[sl_num - 1].get_percentage();
    return true;
}

bool try_handle_stackpercent_numeric(uint8_t sl_num, String & resp) {
    String str_val_token = String(subtokens[1]);
    bool handled = false;
    char *__endptr;
    // strtoul handles  '0x' or decimilar if we give it base==0
    long percentage = strtoul(str_val_token.c_str(), &__endptr, 0);
    if (__endptr[0] == '\0') {  // strtoul sets endptr to last part of #, so /0 means we did the entire string
        if (percentage <= 100) {
            resp += percentage;
            stack_lights[sl_num - 1].set_percentage(percentage);
            handled = true;
        }
    }
    return handled;
}

/**
 * function: handle_SL_worker
 * sl_num:  which stack light are you controlling?  1, 2, or 3
 * appends:  hex color to output display
 * Handle Stack Light worker
 * Expects values to be in hexadecimal notation (e.g. "0x0AFF") or a color in all caps ("RED")
 */
void handle_SLC_worker(uint8_t sl_num) {
    char buff[kCOLORLENGTH];
    char err[MAX_ERROR_STRING_LENGTH];
    bool handled = false;
    String resp("");
    processing_is_ok = true;

    copy_subtoken0colon_into(resp);

    //  valid string lengths:   1: ?     8 hex:  "0x123456"     8 deci for 0xFFFFFF <=:  "16777215"
    // STEP 2:  DO WE HAVE SUBTOKENS
    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;
        handled = true;

        // STEP 2A:   WE GOT SUBTOKENS
    } else if ((sl_num >= 1) && (sl_num <= kNUM_STACKLIGHTS + 1)) {  // this is a triple check actually

        handled = try_handle_stacklight_query(sl_num, resp);

        if (!handled) {
            // check text name first so we don't accidentally convert text to a number...
            handled =  try_handle_stacklight_colorname(sl_num, resp);
        }

        if (!handled) {
            handled = try_handle_stacklight_numeric(sl_num, resp);
        }
    }

    // ...NOT SURE HOW TO PROCESS
    if (!handled) {
        processing_is_ok = false;
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }
    resp_2_output_string(resp);
}

/**
 * Given that we got "SLCx:?"   append appropriate response resp string.
 * @param sl_num    number of our stacklight.  Assumes sl_num already validated for 1...kNUM_STACKLIGHTS
 * @param resp   The String object we are appending our responses to.  Flexing Arduino String class, it's convenient
 */
bool try_handle_stacklight_query(uint8_t sl_num, String &resp) {
    if (strcmp_P(subtokens[1], str_QUERY) != 0) {
        return false;
    }
    char buff[kCOLORLENGTH];
    //  Got  SLC1:?    return   SL1:0xAABBCC:COLOR
    //resp += C2HS(stack_lights[sl_num -1].color);  // "color 2 hex string"
    char *tempstr = new char[UNS_HEX_STR_SIZE];
    uint32_t c = stack_lights[sl_num - 1].get_color();
    color_uint_to_hex_string(c, tempstr, UNS_HEX_STR_SIZE);
    resp += tempstr;
    delete[] tempstr;

    const char * string_in_progmem = (const char *) stack_lights[sl_num - 1].colorname_p;
    strcpy_P(buff, string_in_progmem);
    if (strcmp_P(buff, str_EMPTY) != 0) {
        //we have some color info, append
        strcpy_P(buff, str_COLON);   // "SLCx"  --> "SLCx:"
        resp += buff;
        strcpy_P(buff, string_in_progmem);  // wasteful...should use another var
        resp += buff;
    }
    return true;
}

/**
 * Checks if first chars are "0x" and does the rest....if not, exits quickly
 * @param sl_num   Assumes already in a valid range
 * @param resp   Response we append to
 */
bool try_handle_stacklight_numeric(uint8_t sl_num, String &resp) {
    char buff[kCOLORLENGTH];
    String str_val_token = String(subtokens[1]);
    bool handled = false;
    uint32_t clr = 0;
    char *__endptr;
    // strtoul handles  '0x' or decimal if we give it base==0
    long longcolor = strtoul(str_val_token.c_str(), &__endptr, 0);
    // Use following block if you want to check that entire string is converted to a number.
    // For now we are happy with handling whatever number we can pull out. Ignore the rest.
//    if (__endptr[0] == '\0') {  // strtoul sets endptr to last part of #, so /0 means we did the entire string
//        handled = true;        // full string handled, no extra text.
//        Serial.println("HANDLED THE ENTIRE NUMBER WE DID!");
    if (__endptr[0] != '\0') {
        // We did NOT handle the entire string.
        //Serial.println("Did not handle entire.");
        if (longcolor == 0) {
            // And we got an error back in our conversion...did not start with a number
            //Serial.println("returning FALSE");
            return false;   // < could not handle...get out of here
        }
    }

//    }
    if (longcolor <= 0xFFFFFF) {
        clr = longcolor;
        // convert the value we got BACK into a string and echo     "SLCx:0xFF00FF"
        char *tempstr = new char[UNS_HEX_STR_SIZE];
        color_uint_to_hex_string(clr, tempstr, UNS_HEX_STR_SIZE);
        resp += tempstr;
        delete[] tempstr;

        // Set the value, the base color and the current color value.
        stack_lights[sl_num - 1].set_color(clr);
        handled = true;
    }

    if (handled) {
        bool matched = false;
        // try to match color names
        // Check to see if we have a corresponding color value
        for (uint8_t i = 0; i < kCOLORS_len; i++) {
            if (kCOLORS[i]->is_equal_value(clr)) {
                matched = true;
                stack_lights[sl_num - 1].colorname_p = (char *)kCOLORS[i]->name_p;
                const char *string_in_progmem = (const char *) stack_lights[sl_num - 1].colorname_p;

                strcpy_P(buff, string_in_progmem);
                //Serial.print(F("WE FOUND A MATCHING COLOR: "));
                //Serial.println(buff);

                strcpy_P(buff, str_COLON);   // "SLCx"  --> "SLCx:"
                resp += buff;
                strcpy_P(buff, string_in_progmem);  // wasteful...should use another var
                resp += buff;
            }
        }  // end trying to find match
        if (!matched) {
            stack_lights[sl_num - 1].colorname_p = (char *) str_EMPTY;   // no match, blank it
        }

    }// end color matching
//    Serial.print("Inside handle numeric response now at: ");Serial.println(resp);
    return handled;
}

/**
 * Checks our value token to see if it matches a known color "RED" or "ORANGE".  Case sensitive of course
 * @param sl_num   already pre-checked to be valie
 * @param resp    String response we will append our results to.
 */
bool try_handle_stacklight_colorname(uint8_t sl_num, String &resp){
    char buff[kCOLORLENGTH];
    bool handled = false;
    //Serial.print("TRYING TO HANDLE A COLOR NAME GIVEN: ");Serial.println(subtokens[1]);
    String str_val_token = String(subtokens[1]);
    str_val_token.toUpperCase();
//    String stcut = str_val_token.substring(0, 3);
//    Serial.print("Slice 0 - 3:");Serial.println(stcut);
//    const char *stcutcstr = stcut.c_str();   // convert to a c_str for comparison
//    Serial.print("Is it RED: ");Serial.println( (strcmp_P(stcutcstr, str_RED) == 0) );
    for (uint8_t i=0; i < kCOLORS_len; i++) {
        if (kCOLORS[i]->is_equal_name(str_val_token.c_str())) {
//            // DEBUG SECTION -- left in as it took a while to figure out progmem pointer dereferencing
//            Serial.println("BEHOLD -- WE HAVE A MATCHING COLOR:");
//            Serial.print("kcolor[i] addr: ");Serial.println((uint16_t)(kCOLORS[i]), DEC);
//            Serial.print("namep addr: ");Serial.println((uint16_t)&(kCOLORS[i]->name_p), DEC);
//            Serial.print("val addr: ");Serial.println((uint16_t)&(kCOLORS[i]->value), DEC);
//            Serial.print("value at ->name_p: ");Serial.println((uint16_t)kCOLORS[i]->name_p, DEC);
//            Serial.print("value at ->value: ");Serial.println((uint32_t)kCOLORS[i]->value, HEX);  // << this works
//            // So we know that kCOLORS[i]->name_p  is the addr location of a char array in progmem
//            byte b = (byte) pgm_read_byte_near(kCOLORS[i]->name_p);
//            Serial.print("byte at ->name_p: ");Serial.print(b, DEC);Serial.print("....");Serial.println((char)b);
//            // The above gets us our first letter "R" as decimal 82
            const char * string_in_progmem = (const char *) kCOLORS[i]->name_p;  // << The magic dereference
            strcpy_P(buff, string_in_progmem);
            //Serial.print("->name_p:   ");Serial.println(buff);      // THIS WORKS!!!!

            // Set the value, the base color and the current color value. Add value to response.
            stack_lights[sl_num - 1].set_color(kCOLORS[i]->value);
            //Serial.print("->value:");Serial.println(kCOLORS[i]->value, HEX);

            //char *hexstr = new char[UNS_HEX_STR_SIZE];
            char hexstr[UNS_HEX_STR_SIZE];
            color_uint_to_hex_string(kCOLORS[i]->value, hexstr, UNS_HEX_STR_SIZE);
            resp += hexstr;
            strcpy_P(buff, str_COLON);
            resp += buff;
            //delete(hexstr);      // if you use new, don't forget this or you get weird overruns

            // Set the text, add to response
            stack_lights[sl_num - 1].colorname_p = (char *) kCOLORS[i]->name_p;
            strcpy_P(buff, string_in_progmem);
            resp += buff;


            handled = true;
        }  // end colorname match
    }
    return handled;
}



/**
 * Stack light mode workers handlers.
 */

/**
 * function:  handle_SLM_worker
 * Handle Stack Light Mode worker.    Given input "SLMx:1"  handle the mode set ("1" in this case).
 * See protocol def, but here is the shorthand.
 * NOTE, if you give a funky value it may reset mode to default as we are using atoi and
 * atoi returns 0 for letters.    Other numbers will be ignored.
 * - **input values:**
  - 0:  Off (Same as `SLC:0x0` but retains set color.)
  - 1:  On, Steady State
  - 2:  On, Pulsing.  Option subtoken for full-cycle blink rate in ms.
  - 3:  On, Flashing.  Optional subtoken for full-cycle blink rate in ms.
  SLM1:3:500  would set stack light #1 to flashing mode with a 500ms full cycle blink rate.
 */
void handle_SLM_worker(uint8_t sl_num) {
    bool handled = false;
    char buff[10];
    char err[MAX_ERROR_STRING_LENGTH];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;
        handled = true;

        // WE GOT SUBTOKENS
    } else if ((sl_num >= 1) && (sl_num <= kNUM_STACKLIGHTS + 1)) {  // this is a triple check actually

        handled = try_handle_stackmode_query(sl_num, resp);

        if (!handled) {
            handled = try_handle_stackmode_numeric(sl_num, resp);
        }
    }

    // ...NOT SURE HOW TO PROCESS
    if (!handled) {
        processing_is_ok = false;
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }
    resp_2_output_string(resp);
}

void handle_SLX_worker(uint8_t sl_num) {
    bool handled = false;
    char buff[10];
    char err[MAX_ERROR_STRING_LENGTH];
    String resp("");

    copy_subtoken0colon_into(resp);

    // STEP 2:  DO WE HAVE SUBTOKENS
    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;
        handled = true;

        // WE GOT SUBTOKENS
    } else if ((sl_num >= 1) && (sl_num <= kNUM_STACKLIGHTS + 1)) {  // this is a triple check actually

        handled = try_handle_stacknumled_query(sl_num, resp);

        if (!handled) {
            handled = try_handle_stacknumled_numeric(sl_num, resp);
        }
    }

    // ...NOT SURE HOW TO PROCESS
    if (!handled) {
        processing_is_ok = false;
        strcpy_P(err, str_VALUE_ERROR);
        resp += err;
    }
    resp_2_output_string(resp);
}

bool try_handle_stacknumled_numeric(uint8_t sl_num, String & resp) {
    char buff[10];
    String str_val_token = String(subtokens[1]);
    bool handled = false;
    uint8_t num_leds = 0;
    char *__endptr;
    // strtoul handles  '0x' or decimal if we give it base==0
    long numgiven = strtoul(str_val_token.c_str(), &__endptr, 0);
    // Use following block if you want to check that entire string is converted to a number.
    // For now we are happy with handling whatever number we can pull out. Ignore the rest.
//    if (__endptr[0] == '\0') {  // strtoul sets endptr to last part of #, so /0 means we did the entire string
//        handled = true;        // full string handled, no extra text.
//        Serial.println("HANDLED THE ENTIRE NUMBER WE DID!");
    if (__endptr[0] != '\0') {
        // We did NOT handle the entire string.
        //Serial.println("Did not handle entire.");
        if (numgiven == 0) {
            // And we got an error back in our conversion...did not start with a number
            //Serial.println("returning FALSE");
            return false;   // < could not handle...get out of here
        }
    }
    if (numgiven > 255) {
        num_leds = 255;
    }
    //TODO:   1 -- update our strip  -- can we do this on the fly with new data?
    //   TODO:   FREE, the re-assign new() the strip to take the change immediately
    //TODO:   2 -- write to EEPROM
    //TODO:   3 -- ASK FOR REBOOT??

    return true;
    }


bool try_handle_stackmode_query(uint8_t sl_num, String & resp) {
    if (strcmp_P(subtokens[1], str_QUERY) != 0) {
        return false;
    }
    char buff[8];
    //  Got  SLP1:?    return   SLP1:99
    resp += stack_lights[sl_num - 1].get_mode();

    if ((stack_lights[sl_num - 1].get_mode() == MODE_FLASH) or
        (stack_lights[sl_num - 1].get_mode() == MODE_PULSE) or
        (stack_lights[sl_num - 1].get_mode() == MODE_RAINBOW) ) {
        strcpy_P(buff, str_COLON);
        resp += buff;
        resp += stack_lights[sl_num - 1].get_cycle_ms();
    }
    return true;
}

bool try_handle_stacknumled_query(uint8_t sl_num, String & resp) {
    if (strcmp_P(subtokens[1], str_QUERY) != 0) {
        return false;
    }
    char buff[8];
    //  Got  SLP1:?    return   SLP1:99
    resp += stack_lights[sl_num - 1].numpixels;
    return true;
}


bool try_handle_stackmode_numeric(uint8_t sl_num, String &resp) {
    char buff[8];
    bool handled = false;
    // CHECK MODE FIRST:
    char err[MAX_ERROR_STRING_LENGTH];
    int mode = atoi(subtokens[1]);
    // AT THIS POINT we should have a value to set, e.g. "SLM1:2" gotta convert 2nd token to a number
    // NOTE that atoi returns 0 for just about any non-number
    switch (mode) {
        case MODE_OFF:
        case MODE_STEADY:
        case MODE_FLASH:
        case MODE_PULSE:
        case MODE_RAINBOW:
            resp += mode;    // append our result: "SLM1:3" -> "SLM1:3" but "SLM1:FUN" -> "SLM1:0" due to atoi
            //set # to mode, return string
            // We are assuming sl_num has been doublechecked at this point...or bad bad array access
            stack_lights[sl_num - 1].set_mode(mode);  // sets a flag...
            handled = true;
            break;
        default:   // NOTE ONE OF OUR DEFINED MODES
            handled = false;
            break;
    }
    // IF we handled the mode, check for cycle time
    if ((handled) and (subtokens[2] != NULL) and (strlen(subtokens[2]) != 0)) {
        strcpy_P(buff, str_COLON);
        resp += buff;
        int cycle = atoi(subtokens[2]);
        // The following must fully verify cycle
        if (cycle == 0) {
            // do nothing, atoi returns 0 if it fails or has characters
            strcpy_P(err, str_VALUE_ERROR);
            resp += err;
        } else if (cycle < kMIN_CYCLE_TIME) {
            cycle = kMIN_CYCLE_TIME;
        } else if (cycle > kMAX_CYCLE_TIME) {
            cycle = kMAX_CYCLE_TIME;
        }
        // NOW SET IT
        if (cycle != 0) {
            resp += cycle;
            stack_lights[sl_num - 1].set_cycle_ms(cycle);
        }
    }
    return true;
}

/**
 * This is a debugging method that dumps all the stack light info over the serial port.
 * DO NOT USE in production as response length may change and formatting may change.
 */
void handle_SLINFO_worker() {
    char buff[10];
    String resp("");

    for (uint8_t lightnum = 0; lightnum < kNUM_STACKLIGHTS; lightnum++) {
        stack_lights[lightnum].print_info();
    }
    processing_is_ok = true;

    copy_subtoken0colon_into(resp);

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

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += I2C_Address;
    } else  {
        uint16_t addr = atoi(subtokens[1]);    // atoi is concat'ing, and it's a uint8 so just rolling over.  Maybe this is ok?
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
    resp_2_output_string(resp);
}

/**
 * function: handle_I2B
 * appends:  the I2C Byte count to output display
 * Used to set or query the number of bytes to read/write
 */
void handle_I2B() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

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
    resp_2_output_string(resp);
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

    copy_subtoken0colon_into(resp);

    char hexprefix[3];             // should be: '0x'
    hexprefix[0] = subtokens[1][0];
    hexprefix[1] = subtokens[1][1];
    hexprefix[2] = '\0';

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

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
        // process the command value
        char * hexstring = &subtokens[1][2];
        //Serial.print(F("# sub st1: ")); Serial.print(hexstring); Serial.print(F("   length: ")); Serial.println(strlen(subtokens[1])) - 2);
        if ( (strlen(hexstring) == I2C_Bytes * 2)) {                 // st1 contains '0x', so -2 is what we want
            //uint8_t bytearraylen = (I2C_Bytes * 2) + 1;                         // +1 NULL
            //char bytechars[bytearraylen];
            //Serial.print(F("# bytearraylen: ")); Serial.println(bytearraylen, DEC);
            //st1.toCharArray(bytechars, arraylen, 2);  // get a standard C-string array
            convertHexStringToByteArray(hexstring, I2C_Data);
            // printBytesAsDec(I2C_Data, I2C_Bytes);

            //-----------
            perform_I2C_write();
            //-----------

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
    resp_2_output_string(resp);
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

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

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
    resp_2_output_string(resp);
}

/**
 * function: handle_I2S
 * appends:  the I2C Setting to output display
 * Used to set or query the setting (or 'register') address
 */
void handle_I2S() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else if (strcmp_P(subtokens[1], str_QUERY) == 0)  {
        resp += I2C_Register;
    } else  {
        uint16_t reg = atoi(subtokens[1]);  // Truncates to uint8, so out of range is meaningless
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
    resp_2_output_string(resp);
}

/**
 * function: handle_I2F
 * appends:  the I2C Found devices to output display
 * Used to query for devices on the I2C bus.
 */
void handle_I2F() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

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
    resp_2_output_string(resp);
}




/**
 * function: handle_BLINK
 * appends:  the LED BLINK time to output display
 * Used to blink the Arduino LED for user identification.
 */
void handle_BLINK() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else  {
        blink_time = atoi(subtokens[1]);
        is_blinking = true;
        blink_start = millis();
        last_blink_change = blink_start;
        resp += blink_time;
    }
    resp_2_output_string(resp);
}


/**
 * function:  handle_BEEP
 * Used to set the alarm state.  Operates Independantly of Stack Light 1, but will pulse if SL is pulsing/flashing.
 */
void handle_BEEP() {
    char buff[10];
    String resp("");

    copy_subtoken0colon_into(resp);

    boolean append_cb = false;
    if (resp_err_VALUE_MISSING(resp)) {
        processing_is_ok = false;

    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        if (beep_enabled) {
            strcpy_P(buff, str_ON);
        } else {
            strcpy_P(buff, str_OFF);
        }
        resp += buff;
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_ON) == 0) {
        beep_enabled = true;
        strcpy_P(buff, str_ON);
        resp += buff;
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_OFF) == 0) {
        beep_enabled = false;
        strcpy_P(buff, str_OFF);
        resp += buff;
        append_cb = true;
    } else {                              // "SLA:238"  Got some weird second token other than 1 or 0
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
    resp_2_output_string(resp);
}

void handle_REBOOT() {
    // this is a hard off, jump to 0, and this works
    asm volatile ("jmp 0");
    // NOTE:  the wdt_disable() method just plain didn't work.
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
    Serial.println(F("# SNIPE v4 for Arduino"));
    Serial.println(F("#--------------------------------------------------"));
    Serial.println(F("# Red Byer    github.com/mizraith"));
    Serial.println(F("# VERSION DATE: 7/2/2023"));
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

void printSerialInputInstructions( ) {
    //char * buff[10];
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
    printString_P(str_SID); printString_P(str_SPACE);
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
    Serial.print(F("# Current Station ID (SID): "));
    Serial.println(SID);
    Serial.println(F("#"));
    Serial.println(F("#####DATA#####"));
}


#pragma mark EEPROM helpers
/***************************************************
 ***************************************************
 *   EEPROM HELPERS
 ***************************************************
 ***************************************************/
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

void printBytesAsDec(uint8_t *data, uint8_t len) {
    //Serial.print(F("# ByteArrayAsDec: "));
    for(uint8_t i=0; i<len; i++) {
        Serial.print(data[i], DEC);
        Serial.print(F("  "));
    }
    Serial.println();
}

#pragma mark I2C Helpers
/***************************************************
 ***************************************************
 *   WIRE Helpers
 ***************************************************
 ***************************************************/
/*
 * function: perform_I2C_write
 * Given all the I2C settings (I2A, I2B, I2S, data) perform
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




#pragma clang diagnostic pop