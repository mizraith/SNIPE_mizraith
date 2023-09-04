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
#include <Arduino.h>    // we make good use of String() class
#include <avr/pgmspace.h>
#include <EEPROM.h>
// Uses SCL and SDA pins
#include <Wire.h>
// DOWNLOAD on github.com/mizraith  included for the datetime class.  Wire.h makes the RTCLib happy.
#include"mizraith_DateTime.h"   

// PROJECT LIBRARIES
#include "SNIPE_ExponentialDecay.h"
#include "SNIPE_Strings.h"
#include "SNIPE_ColorUtilities.h"
#include "SNIPE_GeneralUtilities.h"
#include "SNIPE_Conversions.h"

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
void SL_startup_sequence();
void SL_update();
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
void printSerialInputInstructions();
void printSerialDataStart();
// EEPROM HELPERS
void readSIDFromEEPROM();
void writeSIDToEEPROM();
// LOW LEVEL HELPERS
void printBytesAsDec(uint8_t*, uint8_t);
// I2C HELPERS
void perform_I2C_write();
int perform_I2C_write_error();
void perform_I2C_read();
// RAM HELPERS


#pragma mark Application Globals
const DateTime COMPILED_ON = DateTime(__DATE__, __TIME__);
const String CURRENT_VERSION = "003";
const String DESCRIPTION = "SNIPE_for_Arduino";

#pragma mark Pinouts
// PIN OUTS
#define ANALOG_INPUT   A0
#define LED_PIN        13
#define RELAY_PIN      6
#define SL1_PIN        5    // 7 <<< SHOULD BE 7.  Using 5 for debug only.
#define SL2_PIN        8
#define SL3_PIN        9
#define SLA_PIN      A6
//#define NUMPIXELS      8   // number of pixels per stacklight.  Nominal 24.

// min and max full cycle time for our flash or pulse modes.
#define kMIN_CYCLE_TIME 100
#define kMAX_CYCLE_TIME 5000

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

#pragma mark Stack Light Variables
//default mode is solid
#define MODE_DEFAULT 0
#define MODE_FLASH   1
#define MODE_PULSE   2
//#define MODE_CYLON_RING  3
// text based colors ... would be cool tos support
#define RED  0xFF0000
#define ORANGE 0xFF5500
#define YELLOW 0xFFFF00
#define GREEN 0x00FF00
#define AQUA 0x00FFFF
#define BLUE 0x0000FF
#define INDIGO 0x3300FF
#define VIOLET 0xFF00FF
#define WHITE  0xFFFFFF
#define BLACK  0x000000

// Parameter 1 = number of pixels in the strip.
// Parameter 2 = SLx_PIN ....Digital pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip1)
//#ifdef USE_NEOPIXEL_LEDS
Adafruit_NeoPixel SL1_strip = Adafruit_NeoPixel(8, SL1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel SL2_strip = Adafruit_NeoPixel(8, SL2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel SL3_strip = Adafruit_NeoPixel(8, SL3_PIN, NEO_GRB + NEO_KHZ800);
//# endif //USE_NEOPIXEL_LEDS


// STACK LIGHT CONTROLLER supporting variables
class StackLight {
public:                         // Access specifier
    uint32_t color {BLACK};         // default 0 value
    uint8_t mode {MODE_DEFAULT};    // solid state
    uint16_t cycle_ms {500};        // 500 ms full cycle time
    unsigned update_time {0};       // immediate
    bool flash_is_on {true};
    bool pulse_going_up {true};
    uint8_t numpixels {8};         // how many pixes in this stack light
    uint8_t perc_lit {100};         // single digit, no floats please
    Adafruit_NeoPixel * strip;
    void change_mode(uint8_t new_mode) {
        // handle mode switches.  Where we want to keep the color on even when flash/pulse has changed
        return;
    }
//    void init_strip() {
//        strip = new(Adafruit_NeoPixel(numpixels, SL1_PIN, NEO_GRB + NEO_KHZ800));
//    }
};

#define kNUM_STACKLIGHTS 3
StackLight stack_lights[kNUM_STACKLIGHTS];   // our array of classes.

unsigned long SL_loop_time = 0;
unsigned long SL_next_heartbeat = 0;        //  Timer for our next refresh
#define kHEARTBEAT_INTVL_ms 50

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
    pinMode(SLA_PIN,OUTPUT);
    pinMode(2,  OUTPUT);
    pinMode(3,  OUTPUT);
    pinMode(4,  OUTPUT);
    pinMode(5,  OUTPUT);
    pinMode(6,  OUTPUT);
    pinMode(SL1_PIN, OUTPUT);
    pinMode(SL2_PIN, OUTPUT);
    pinMode(SL3_PIN,OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(SLA_PIN, LOW);
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(LED_PIN, LOW);

    printSerialInputInstructions();
    printSerialDataStart();
    SL_loop_time = millis();
    SL_next_heartbeat = millis();    // Set to now

    for(StackLight light : stack_lights) {
        light.color = 0;
        light.mode = 0;
        light.cycle_ms = 500;
        light.flash_is_on = true;
    }
    stack_lights[0].strip = &SL1_strip;
    stack_lights[1].strip = &SL2_strip;
    stack_lights[2].strip = &SL3_strip;

    SL_startup_sequence();
}


/***************************************************
 ***************************************************
 *   MAIN LOOP -- Arduino style
 ***************************************************
 ***************************************************/
void loop() {
    // put your main code here, to run repeatedly:
    A_values[0] = analogRead(A0);
    A_values[1] = analogRead(A1);
    A_values[2] = analogRead(A2);
    A_values[3] = analogRead(A3);

    // TODO:  PICKUP HERE.......................
    if (millis() > SL_next_heartbeat) {
        SL_next_heartbeat = millis() + kHEARTBEAT_INTVL_ms;
        SL_update();
    }

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

void stack_light_flash_worker() {
    unsigned long current_time = millis();

    // for each stack light, if flash is set, see if it is time to toggle.
    for (int i=0; i < kNUM_STACKLIGHTS; i++) {
        if (stack_lights[i].mode == MODE_FLASH) {
            // if time then toggle?
            // toggle state.
        }
    }

    //for(uint8_t & SL_Mode : SL_Modes) {

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
    //long time_start = millis();

    //Note that the ">" has already been removed by this point.
    processing_is_ok = true;  // leave as true....only set false if we encounter an error

    //Serial.print(F("# RECEIVED_INPUT->"));
    //Serial.println(input_string);

    char* token;
    char* tempstring;
    char* stringtofree;

    output_string = "";                 // clear it

    tempstring = strdup(input_string);  // does arduino support strdup() ?
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

    //long time_end = millis();
    //long totaltime = time_end - time_start;
    //Serial.print(F("# Total execution (ms): "));
    //Serial.println(totaltime, DEC);
    //checkRAMandExitIfLow(11);
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
    // SPLIT INTO SUBTOKENS AND STORE IN OUR GLOBAL ARRAY, so we don't have to do the work again
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
    } else if (strcmp_P(subtokens[0], str_SLC1) == 0) {
        handle_SLC1();
    } else if (strcmp_P(subtokens[0], str_SLC2) == 0) {
        handle_SLC2();
    } else if (strcmp_P(subtokens[0], str_SLC3) == 0) {
        handle_SLC3();
    } else if (strcmp_P(subtokens[0], str_SLM1) == 0) {
        handle_SLM1();
    } else if (strcmp_P(subtokens[0], str_SLM2) == 0) {
        handle_SLM2();
    } else if (strcmp_P(subtokens[0], str_SLM3) == 0) {
        handle_SLM3();
    } else if (strcmp_P(subtokens[0], str_SLA) == 0) {
        handle_SLA();
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
}

void handle_A1() {
    handle_A_worker(1);
}

void handle_A2() {
    handle_A_worker(2);
}

void handle_A3() {
    handle_A_worker(3);
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
    switch (numA) {
        case 0 ... (kNUM_ANALOG_PINS - 1):
            strcpy_P(buff, str_A);
            resp += buff;
            resp += numA;  // "A0" make use of that Arduino String += operation
            val = A_values[numA];   // 0 index lines up nice
            break;
        default:
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_VALUE_ERROR);
            port = err;
            val = 0;
            break;
    }
    strcpy_P(buff, str_COLON);  //   resp --> "Ax:"
    resp += buff;

    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {
        processing_is_ok = false;
        char err[MAX_ERROR_STRING_LENGTH];
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;
    } else if (strcmp_P(subtokens[1], str_QUERY) == 0) {
        resp += val;                  // Arduino String allows concat of an int, but must be on its own line
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
}

void handle_D3() {
    handle_D_worker(3);
}

void handle_D4() {
    handle_D_worker(4);
}

void handle_D5() {
    handle_D_worker(5);
}

void handle_D6() {
    handle_D_worker(6);
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
        case 2 ... 6:
            strcpy_P(buff, str_D);
            resp += buff;
            resp += numpin;   // "D4"
            break;
        default:
            char err[MAX_ERROR_STRING_LENGTH];
            strcpy_P(err, str_VALUE_ERROR);
            break;
    }
    strcpy_P(buff, str_COLON);
    resp += buff;                // "Dx:"

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

void SL_startup_sequence() {
    uint32_t wash_colors[5] = {RED, YELLOW, GREEN, BLUE, BLACK};
    for(unsigned long wash_color : wash_colors) {
        for (uint8_t i = 0; i < 255; i++) {
            for (StackLight light: stack_lights) {
                i = min(i, light.numpixels - 1);
                light.strip->setPixelColor(i, wash_color);
                light.strip->show();
            }
        }
        delay(100);
    }
}

void SL_update() {
    // TODO:  How to handle flash and strobe.
    uint8_t numactive;

    for(StackLight light : stack_lights) {
        numactive = int((float)light.numpixels * ((float)light.perc_lit / 100));
        brightness = calc_step_brightness(&light);
    }
}



void handle_SLC1() {
    handle_SLC_worker(1);
}

void handle_SLC2(){
    handle_SLC_worker(2);
}
void handle_SLC3(){
    handle_SLC_worker(3);
}

/**
 * function: handle_SL_worker
 * sl_num:  which stack light are you controlling?  1, 2, or 3
 * appends:  hex color to output display
 * Handle Stack Light worker
 * Expects values to be in hexadecimal notation (e.g. "0x0AFF")
 * TODO:  Someday we'll handle basic TEXT string color inputs as well.
 */
void handle_SLC_worker(uint8_t sl_num) {
    char buff[10];
    char err[MAX_ERROR_STRING_LENGTH];
    String resp("");
    // Step 1:  Add "SLCx" to response
    switch(sl_num) {
        case 1 ... kNUM_STACKLIGHTS:
            strcpy_P(buff, str_SLC);
            resp = buff;
            resp += sl_num;      // leveraging that String class.
            break;
        default:
            strcpy_P(err, str_VALUE_ERROR);
            resp += err;
            break;
    }

    strcpy_P(buff, str_COLON);   // "SLCx"  --> "SLCx:"
    resp += buff;

    // STEP 2:  DO WE HAVE SUBTOKENS
    if ( (subtokens[1] == NULL ) || (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token, e.g. "SLC:" or "SLC"
        processing_is_ok = false;
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;   // then we will skip the other processing.
    // STEP 2A:   WE GOT SUBTOKENS
    } else {
        //  STEP 2A   DID WE GET A QUERY?    append our color to the response:   SLCx:0xFF00FF
        if (strcmp_P(subtokens[1], str_QUERY) == 0) {
            switch (sl_num) {
                case 1 ... kNUM_STACKLIGHTS:
                    resp += C2HS(stack_lights[sl_num -1].color);  // "color 2 hex string"
                    break;
                default:
                    //we already caught this issue earlier
                    break;
            }
        }
        // STEP 2B  WE GOT SOME VALUE -- HOPEFULLY HEX
        // AT THIS POINT we should have a value to set, e.g. "SLC1:0x33ff00" gotta convert that to a number
        char hexprefix[3];             // hexprefix should be: '0x'
        hexprefix[0] = subtokens[1][0];
        hexprefix[1] = subtokens[1][1];
        hexprefix[2] = '\0';      // c-string terminator
        // VERIFY WE GOT A HEX STRING....HANDLE THAT TOKEN subtoken[1] == "0xYYYYYY"
        if (strcmp_P(hexprefix, str_HEX) == 0) {     // verify we start with 0x
            char *hexstring = &subtokens[1][2];      // from "0x00FF00" --> "00FF00"
            //Serial.print(F("# sub st1: ")); Serial.print(hexstring); Serial.print(F("   length: ")); Serial.println(strlen(subtokens[1])) - 2);
            uint32_t clr;
            clr = color_uint_from_hex_string(hexstring);
            // convert the value we got BACK into a string and echo     "SLCx:0xFF00FF"
            resp += C2HS(clr);
            // But don't forget to set teh value
            switch (sl_num) {
                case 1 ... kNUM_STACKLIGHTS:
                    stack_lights[sl_num - 1].color = clr;
                    break;
                default:
                    //we've already caught this issue earlier
                    break;
            }
        // STEP 2C:  NOT A HEX STRING...NOT SURE HOW TO PROCESS
        } else {                // got some weird token lacking a "0x" up front...SLC1:888  we just won't handle it
            processing_is_ok = false;
            char valerr[MAX_ERROR_STRING_LENGTH];
            strcpy_P(valerr, str_VALUE_ERROR);
            resp += valerr;
        }
    }

    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
}

/**
 * Stack light mode workers handlers.
 */
void handle_SLM1(){
    handle_SLM_worker(1);
}

void handle_SLM2(){
    handle_SLM_worker(2);
}

void handle_SLM3(){
    handle_SLM_worker(3);
}

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
    char buff[10];
    char err[MAX_ERROR_STRING_LENGTH];
    String resp("");
    // Step 1:  Add "SLMx" to response
    switch (sl_num) {
        case 1 ... kNUM_STACKLIGHTS:
            strcpy_P(buff, str_SLM);
            resp = buff;
            resp += sl_num;   // leverage the Arduino String class
            break;
        default:
            strcpy_P(err, str_VALUE_ERROR);
            resp += err;
            break;
    }

    strcpy_P(buff, str_COLON);    // "SLMx"  --> "SLMx:"
    resp += buff;

    // STEP 2:  DO WE HAVE SUBTOKENS
    if ((subtokens[1] == NULL) ||
        (strlen(subtokens[1]) == 0)) {  // didn't give us a long enough token, e.g. "SLM:" or "SLM"
        processing_is_ok = false;
        strcpy_P(err, str_VALUE_MISSING);
        resp += err;   // then we will skip the other processing.
        // STEP 2A:   WE GOT SUBTOKENS
    } else {
        //  STEP 2A   DID WE GET A QUERY?    append our mode to the response:   SLMx:1
        if (strcmp_P(subtokens[1], str_QUERY) == 0) {
            switch (sl_num) {
                case 1 ... kNUM_STACKLIGHTS:
                    resp += stack_lights[sl_num - 1].mode;
                    strcpy_P(buff, str_COLON);
                    resp += buff;
                    resp += stack_lights[sl_num - 1].cycle_ms;
                    break;
                default:
                    //we already caught this issue earlier
                    break;
            }
        }
        // STEP 2B  WE GOT SOME VALUE, HOPEFULLY IN OUR RANGE
        // AT THIS POINT we should have a value to set, e.g. "SLM1:2" gotta convert 2nd token to a number
        // NOTE that atoi returns 0 for just about any non-number
        int mode = atoi(subtokens[1]);
        switch (mode) {
            case MODE_DEFAULT:
            case MODE_FLASH:
            case MODE_PULSE:
                resp += mode;    // append our result: "SLM1:3" -> "SLM1:3" but "SLM1:FUN" -> "SLM1:0" due to atoi
                //set # to mode, return string
                switch (sl_num) {
                    case 1 ... kNUM_STACKLIGHTS:
                        // TODO:  --> control state change.
                        stack_lights[sl_num - 1].mode = mode;
                        // TODO:  If we are switching from Pulse or Flash -> Normal we need to set enable.

                        break;
                    default:
                        //we caught this earlier
                        break;
                }
                break;
            default:   // NOTE ONE OF OUR DEFINED MODES
                strcpy_P(err, str_VALUE_ERROR);
                resp += err;
                break;
        }
    }
    // STEP 3:  Check if we were given an OPTIONAL pulse rate and round up or down to our acceptable range.
    if ((subtokens[2] != NULL) && (strlen(subtokens[2]) != 0)) {
        // update
        int cycle = atoi(subtokens[2]);
        // The following must fully verify cycle
        if (cycle == 0) {
            // do nothing
            strcpy_P(err, str_VALUE_ERROR);
            resp += err;
        } else if (cycle < kMIN_CYCLE_TIME) {
            cycle = 100;
            resp += cycle;
        } else if (cycle > kMAX_CYCLE_TIME) {
            cycle = 5000;
            resp += cycle;
        }
        // NOW SET IT
        if (cycle != 0) {
            switch (sl_num) {
                case 1 ... kNUM_STACKLIGHTS:
                    stack_lights[sl_num - 1].cycle_ms = cycle;
                    break;
                default:
                    //we caught this earlier
                    break;
            }
        }
    }
    // STEP 4....we are done!
    strcpy_P(buff, str_SPACE);
    resp += buff;
    output_string.concat(resp);
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
        resp += digitalRead(SLA_PIN);
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_ON) == 0) {
        digitalWrite(SLA_PIN, HIGH);
        resp += digitalRead(SLA_PIN);
        append_cb = true;
    } else if (strcmp_P(subtokens[1], str_OFF) == 0) {
        digitalWrite(SLA_PIN, LOW);
        resp += digitalRead(SLA_PIN);
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
    hexprefix[2] = '\0';

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
        // process the command value
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
    hexprefix[2] = '\0';

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