///**
// * Part of SNIPE
// *
// * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
// * See License file in PPC_Main.h
// */
//
//
//#include "SNIPE_Modes.h"
//
////  ARDUINO DEVELOPERS -- the Arduino linker is stupid so this #define is in 2 places
////  CLion CMake DEVELOPERS (if compiled on CLion, you need to use the CMakeLists.txt flags instead)
////#define USE_NEOPIXEL_LEDS
////#define USE_WS2801_LEDS
//
//
//
////prototypes for external functions found in our main .ino file
//void activeDelay( uint32_t );
//
//
//#pragma mark MODE LOGIC
////*******************************************************************************************
////*******************************************************************************************
////
////                                    MODE LOGIC
////
////*******************************************************************************************
////*******************************************************************************************
////valid modes are 1 -> NUMBER OF MODES
//void incrementMode( ) {
//    if (CURRENT_MODE >= kNUMBER_OF_MODES) {
//        CURRENT_MODE = 1;
//    } else {
//        CURRENT_MODE++;
//    }
//    mode_did_change = true;     //SET OUR FLAG
//
//    #ifdef DEBUG_MODE
//    Serial.print(F("Incrementing to mode: "));
//    Serial.println(CURRENT_MODE, DEC);
//    #endif //DEBUG MODE
//}
//
//
////valid modes are 1 -> NUMBER OF MODES
//void
//decrementMode( ) {
//    if (CURRENT_MODE <= 1 ) {
//        CURRENT_MODE = kNUMBER_OF_MODES;
//    } else {
//        CURRENT_MODE--;
//    }
//    mode_did_change = true;     //SET ORU FLAG
//    #ifdef DEBUG_MODE
//    Serial.print(F("Decrementing to mode: "));
//    Serial.println(CURRENT_MODE, DEC);
//    #endif //DEBUG MODE
//}
//
//
//
//void handleNormalModeEncoderChange( ) {
//    uint8_t newval;
//    switch (NORMAL_MODE_SELECTED_SETTING) {
//        case kIDX_NONE:
//            accumulated_encoder_count = 0;
//            return;
//        case kIDX_POT1:
//            newval = POT_VALUE_1 + accumulated_encoder_count;
//            if (newval < 1) { newval = 1; }
//            else if (newval > kMAX_POT_VALUE) { newval = kMAX_POT_VALUE; }
//            POT_VALUE_1 = newval;
//            break;
//        case kIDX_POT2:
//            newval = POT_VALUE_2 + accumulated_encoder_count;
//            if (newval < 1) { newval = 1; }
//            else if (newval > kMAX_POT_VALUE) { newval = kMAX_POT_VALUE; }
//            POT_VALUE_2 = newval;
//            break;
//        case kIDX_POT3:
//            newval = POT_VALUE_3 + accumulated_encoder_count;
//            if (newval < 1) { newval = 1; }
//            else if (newval > kMAX_POT_VALUE) { newval = kMAX_POT_VALUE; }
//            POT_VALUE_3 = newval;
//            break;
//        default:
//            break;
//    }
//    pots_did_change = true;   // drive the display update
//    PPC_settings_did_change = true; //drive the mode to update calculated values
//    accumulated_encoder_count = 0;
//
//
//}
//
//
//// Encoder has an accumulated count and we are in the pixel pacer mode.
//void handlePPCEncoderChange( ) {
//    uint8_t newval;
//    uint16_t newpace;
//    switch (PPC_SELECTED_SETTING) {
//        case kIDX_NONE:
//            accumulated_encoder_count = 0;
//            return;
//            // THIS IS WHERE WE COULD ask user if they want to switch out of Pixel Pacer mode?
//            // OR require a full spin of encoder (big accumulated count) to change out
//        case kIDX_LENGTHS:
//            newval = LENGTHS + accumulated_encoder_count;
//            if (newval < kMIN_LENGTHS) { newval = kMIN_LENGTHS; }
//            else if (newval > kMAX_LENGTHS) { newval = kMAX_LENGTHS; }
//            LENGTHS = newval;
//            PACE = (uint16_t)(LENGTHS * PACE_PER_LENGTH);   // recalculate pace, but round
//            break;
//        case kIDX_PACE:
//            newpace = PACE + accumulated_encoder_count;
//            if (newpace < kMIN_PACE) { newpace = kMIN_PACE; }
//            else if (newpace > kMAX_PACE) { newpace = kMAX_PACE; }
//            PACE = newpace;
//            PACE_PER_LENGTH = ((float)PACE) / LENGTHS;     // recalculate pace per length
//            break;
//        case kIDX_REST:
//            newval = REST + accumulated_encoder_count;
//            if (newval < kMIN_REST) { newval = kMIN_REST; }
//            else if (newval > kMAX_REST) { newval = kMAX_REST; }
//            REST = newval;
//            break;
//        case kIDX_SWIMMERS:
//            newval = SWIMMERS + accumulated_encoder_count;
//            if (newval < kMIN_SWIMMERS) { newval = kMIN_SWIMMERS; }
//            else if (newval > kMAX_SWIMMERS) { newval = kMAX_SWIMMERS; }
//            SWIMMERS = newval;
//            break;
//        case kIDX_INTERVAL:
//            newval = INTERVAL + accumulated_encoder_count;
//            if (newval < kMIN_INTERVAL) { newval = kMIN_INTERVAL; }
//            else if (newval > kMAX_INTERVAL) { newval = kMAX_INTERVAL; }
//            INTERVAL = newval;
//            break;
//        default:
//            break;
//    }
//    pots_did_change = true;   // drive the display update
//    PPC_settings_did_change = true; //drive the mode to update calculated values
//    accumulated_encoder_count = 0;
//}
//
//
//
//
//// Push the ball forward.
//// This is called every loop in order to call our active function.
//void updateCurrentMode() {
//    if ( CURRENT_MODE == kMODE_PROGRAMMING) {      // We could put a hook here to call PixelPacer if we don't want it to be a contiguous mode number
//        doProgrammingMode();
//        return;
//    }
//
//    //create a scratchpad mode structure for getting data out of PROGMEM.  Be sure to dealloc or we'll get a memory leak
//    auto * scratchMode =  new struct mode;
//    //get the address of our mode structure in PROGMEM ---------------- subtract 1 from index as we are going directly into our array
//    auto * modeAddressInProgmem = (mode *)pgm_read_word_near( &(MASTER_MODE_TABLE[CURRENT_MODE - 1]) );
//    //Do some pointer math to get the pointer to our mode's function.
//    scratchMode->func_p = (mode_func *) pgm_read_word_near( ((uint16_t)modeAddressInProgmem) + kMODE_INDEX_FUNCTION);
//    //Call our current mode -- also used to launch app
//    scratchMode->func_p();
//
//    delete(scratchMode);
//}
//
//
//// COMMENTED OUT for PPC as it is unused
////void serialPrintInputValues() {
////    get8BitInputPotValues();
////
////    Serial.print(F("CURRENT MODE: "));
////    Serial.print(CURRENT_MODE, DEC);
////    Serial.print(F("     8-bit values [1,2,3]:\t"));
////    Serial.print(POT_VALUE_1, DEC);
////    Serial.print(F("\t"));
////    Serial.print(POT_VALUE_2, DEC);
////    Serial.print(F("\t"));
////    Serial.println(POT_VALUE_3, DEC);
////}
//
//
//
//#pragma mark MODES
////*******************************************************************************************
////*******************************************************************************************
////*******************************************************************************************
////*******************************************************************************************
////
////          MODES
////
////  Modes are transient.  They must be able to start from scratch and can only
////  maintain state through predefined variables.
////     Therefore a mode must:
////        - Check to see if the "mode_did_change" and if so, initialize itself and update shared variables
////            ONLY THEN does the mode clear the "mode_did_change" variable.
////        - Do the next step in calculations, then exit and resume -- OR --
////        - Keep checking on mode_did_change and break when true, making sure
////          to frequently update the OLED display.
////
////  Note that modes make use of activeDelay which is a short delay function
////  that regualrly checks the states of the pots.
////
////*******************************************************************************************
////*******************************************************************************************
////*******************************************************************************************
////*******************************************************************************************
//
////prototypes to support programming mode
//void displayInitialProgrammingModeScreen();
//void displayUpdateProgrammingMode( );
//
//
///**
// * ----------doProgrammingMode()-----------------------
// *     In this mode we will display the options and allow the user to
// *  adjust the values.  A delay of 5 seconds without input will exit
// *  the programmign mode.
// **/
//void doProgrammingMode() {
//    //unsigned long last_user_action = my_millis();
//    time_any_int = my_millis();          //reuse as last_user_action to save us some precious SRAM
//    uint8_t sequence = 1;
//    uint16_t old_NumPixels = NUMPIXELS;
//
//    CURRENT_MODE = kMODE_PROGRAMMING; //double check
//    mode_did_change = false;
//    Serial.println(F("@@@ Advanced Settings @@@"));
//    displayInitialProgrammingModeScreen();
//    progColorWipe(sequence++);
//
//    delay(2000);       //let it sink in
//
//    while( (my_millis() - time_any_int) < 10000) { //(2 * kTIME_DELAY_LONG_BUTTON_PRESS)) {
//        displayUpdateProgrammingMode();
//        progColorWipe(sequence++);
//        //If we had different modes we would run a switch and respond differently
//        //NUMBER OF LED MODE -- break this out if we add more programming modes
//        if(accumulated_encoder_count != 0) {
//            time_any_int = my_millis();        //our last user action
//            uint16_t newNumPixels = NUMPIXELS + accumulated_encoder_count;
//            accumulated_encoder_count = 0;  //clear it
//
//            if( (newNumPixels < 1) ||    // make sure it's positive, then cast it for check
//                ( (accumulated_encoder_count > 0) && ((uint8_t)accumulated_encoder_count >= NUMPIXELS))
//                    ) {
//                newNumPixels = 1;
//            }
//            if( (newNumPixels > kMAX_NUMPIXELS) ) {
//                newNumPixels = kMAX_NUMPIXELS;
//            }
//
//            if(newNumPixels != NUMPIXELS) {
//                Serial.print(F("New num pixels: "));
//                Serial.println(newNumPixels, DEC);
//                NUMPIXELS = newNumPixels;
//                setMaxNumPixelsToColor( npBLACK );   //be sure to blank it all
//            }
//        }
////        //TODO:  how to sense another button push to EXIT programming mode
////        if(button_do_service_INT[kIDX_BUTTON_ENCODER]) {
////            button_do_service_INT[kIDX_BUTTON_ENCODER] = false;
////            //future: could use this to change what we are programming
////            break;
////        }
//    }
//    if (NUMPIXELS != old_NumPixels) {
//        //save to EEPROM
//        doSaveNumPixelsToEEPROM();
//    }
//    CURRENT_MODE = LAST_MODE;
//    mode_did_change = true;
//}
//
//
//
///**
// * ------------- doSolidColor() -------------------------
// * You can use this as a template mode for a single-pass method
// *
// *    display a single color on entire strip
// *    POT1:  sets R value from 0-255
// *    POT2:  sets G value from 0-255
// *    POT3:  sets B value from 0-255
// **/
//void doSolidColor() {
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//    uint32_t color;
//    while( !mode_did_change ) {
//        //Index into our exponential table to get LED drive value.
//        color = getColorFromRGB( POT_VALUE_1, POT_VALUE_2, POT_VALUE_3 );
//        //do one pass and exit
//        for(uint16_t i=0; i < NUMPIXELS; i++) {
//            strip.setPixelColor(i, color);
//        }
//        strip.show();
//        activeDelay(100);  // allow background tasks time to complete
//    }
//}
//
//
///**
// * ------------- doPulseColor() --------------------
// *    pulses (on/off) a single color on the entire strip.
// *    POT1:  Sets Hue  (from R-G-B)
// *    POT2:  Sets brightness (from 1% -> 100%)
// *    POT3:  Sets strobe rate, from 0.25Hz to 20Hz
// **/
//void doPulseColor() {
//    bool goingup = true;
//    uint8_t halfcyclenumber = 0;          //state
//    uint16_t refreshdelay; // = 50;            //in ms
//    uint8_t numhalfcyclesteps; // = 40;
//    uint8_t peakbrightness; // = 255;
//    uint8_t stepsize = 0;
//    //uint8_t currentoffset = 0;
//    //uint16_t rawval = 0;          //storage for pot reads
//    uint32_t color; // = 0x000000;
//
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//
//    while(!mode_did_change) {
//        //COLOR -- POT 1
//        //color = getRGBHueFromPot( 1 );
//        color = getColorFrom8BitValue(POT_VALUE_1);
//
//        //CYCLE RATE -- POT 3
//        //     cycletime can go from 4192 (10s) down to 100 (1/4s). Note that 10000 - (1023 * 10) = 250
//        // -- NO LONGER USING POTS  rawval = analogRead(POT_3_PIN);  //10-bit value value from 0 to 1023
//        // -- NO LONGER USING POTS  uint16_t halfcycletime = 4192 - (rawval * 4);      //  4192 > halfcycletmime > 100
//        uint16_t halfcycletime = 4192 - ((uint16_t)POT_VALUE_3 * 16);  // 4176 > halfcycletime > 112
//
//
//        //REFRESH DELAY
//        refreshdelay = getVariableRefreshDelayFor(halfcycletime);
//        //STEPS      we keep this to < 255 -- don't need that many
//        numhalfcyclesteps = (uint8_t) (halfcycletime / refreshdelay);
//
//        //MAX BRIGHTNESSS -- POT_VALUE_2
//        //      0 = min brightness at peak     255 = max brightness at peak
//        //      ontime = (POT_VALUE_2 / 1023) * cycletime
//        peakbrightness = POT_VALUE_2;
//        //PEAK BRIGHTNESS LOWER LIMIT
//        if (peakbrightness < 10) {
//            peakbrightness = 10;      // so we always get light out, determined empircally. If you go too low, the strobe stutters
//        }
//        //STEP SIZE
//        //Given:   ( peakbrightness / halfcycletime ) == ( stepsize / refreshdelay )
//        //        stepsize = refreshdelay ( peakbrightness / halfcycletime)
//        // THE CHALLENGE is that on the extremes our step size is too small.  We can increase this
//        // by gradually ratcheting up the refreshdelay (makes sense for longer pules cycle times).
//        stepsize =   (uint8_t) (((uint16_t)refreshdelay * (uint16_t)peakbrightness ) / halfcycletime );
//        uint8_t x = 1;
//        while( (stepsize < 1) && (x <= 8)) {
//            refreshdelay = refreshdelay *  x;
//            stepsize =   (uint8_t) ((refreshdelay * peakbrightness ) / halfcycletime );
//            x++;
//        }
//        if (stepsize < 1) { stepsize = 1; }
//
//        if(goingup) {
//            halfcyclenumber++;
//        } else {
//            halfcyclenumber--;
//        }
//        //CHECK FOR DIRECTION FLIP
//        if(halfcyclenumber >= numhalfcyclesteps) {
//            goingup = false;
//            halfcyclenumber = numhalfcyclesteps;
//        }
//        if(halfcyclenumber == 0) {
//            goingup = true;
//        }
//
//        uint8_t brightness = (uint8_t)(stepsize * halfcyclenumber);
//        color = colorWithBrightness( color, brightness );
//
//        //UPDATE PIXELS
//        for(uint16_t i=0; i < NUMPIXELS; i++) {
//            strip.setPixelColor(i, color);
//        }
//        strip.show();
//        activeDelay(refreshdelay);
//    }
//}
//
///**
// *  ---------------- doStrobeColor() --------------------
// *   strobes (on/off) a single color on the entire strip.
// *    POT1:  Sets Hue  (from R-G-B)
// *    POT2:  Sets duty cycle (from 1% -> 100%)
// *    POT3:  Sets strobe rate, from 0.25Hz to 20Hz
// **/
//void doStrobeColor() {
//    uint16_t cycletime; // =  1000;    //in ms....max 8000
//    uint16_t ontime; // = 500;
//    uint16_t offtime; // = cycletime - ontime;
//    //uint16_t rawval = 0;          //storage for pot reads
//
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//
//    while(!mode_did_change) {
//        //COLOR -- POT 1
//        //uint32_t color = getRGBHueFromPot( 1 );
//        uint32_t color = getColorFrom8BitValue(POT_VALUE_1);
//
//        //STROBE RATE -- POT 3
//        //     cycletime can go from 2100 (~1/4Hz) down to 54 (~20Hz). Note that 2100 - (1023 * 2) = 54
//        // -- NO POTS IN PPC rawval = analogRead(POT_3_PIN);  //10-bit value value from 0 to 1023
//        // -- NO POTS IN PPC cycletime = 2100 - (rawval * 2);  //  2100 > cycletime > 52
//        cycletime = 2100 - ((uint16_t)POT_VALUE_3 * 8);  //  2092 > cycletime > 60
//
//        //  This would be better logarithmically
//
//        //DUTY CYCLE -- POT 2
//        //      0 = duty cycle (on/off) of .01    255 = duty cycle (on/off) = 1.0
//        //      ontime = (POT_VALUE_2 / 1023) * cycletime
//        ontime = ( POT_VALUE_2 * (uint32_t)cycletime) / 255;
//        //DUTY CYCLE LIMITS
//        if (ontime < 4) {
//            ontime = 4;      // so we always get light out, determined empircally. If you go too low, the strobe stutters
//        }
//        if (ontime > (cycletime - 4)) {
//            ontime = cycletime - 4;        //so we always see a blip, determined empircally
//        }
//        offtime = cycletime - ontime;
//
//        //UPDATE PIXELS
//        for(uint16_t i=0; i < NUMPIXELS; i++) {
//            strip.setPixelColor(i, color);
//        }
//        strip.show();
//        activeDelay(ontime);
//
//        for(uint16_t i=0; i < NUMPIXELS; i++) {
//            strip.setPixelColor(i, npBLACK);
//        }
//        strip.show();
//        activeDelay(offtime);
//    }
//}
//
//
///**
// *  ---------------- doCrossFade() --------------------
// *    cross-fades alternating LEDs on strip
// *    POT1:  Sets Hue 1  (from R-G-B)
// *    POT2:  Sets Hue 2
// *    POT3:  Sets cycle rate, from 0.25Hz to 20Hz
// **/
//void doCrossFade() {
//    bool goingup1 = true;             //track primary, secondary is opposite
//    uint8_t halfcyclenumber = 0;          //state
//    uint16_t refreshdelay; // = 50;            //in ms
//    uint8_t numhalfcyclesteps; // = 40;
//    uint8_t peakbrightness = 250;      //don't really need max brightness
//    uint8_t stepsize = 0;
//    //uint8_t currentoffset = 0;
//    //uint16_t rawval = 0;          //storage for pot reads
//    uint32_t color1; // = 0x000000;
//    uint32_t color2; // = 0x000000;
//
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//
//
//    while(!mode_did_change) {
//        //COLOR -- POT 1 & 2
//        //color1 = getRGBHueFromPot( 1 );
//        //color2 = getRGBHueFromPot( 2 );
//        color1 = getColorFrom8BitValue(POT_VALUE_1);
//        color2 = getColorFrom8BitValue(POT_VALUE_2);
//
//
//        //CYCLE RATE -- POT 3
//        //     cycletime can go from 4192 (10s) down to 100 (1/4s). Note that 10000 - (1023 * 10) = 250
//        // -- NO POTS IN PPC  rawval = analogRead(POT_3_PIN);  //10-bit value value from 0 to 1023
//        // -- NO POTS IN PPC  uint16_t halfcycletime = 4192 - (rawval * 4);      // 4192 > halfcycletime > 100
//        uint16_t halfcycletime = 4192 - ((uint16_t)POT_VALUE_3 * 16);      // 4176 > halfcycletime > 112
//
//
//        //REFRESH DELAY
//        refreshdelay = getVariableRefreshDelayFor(halfcycletime);
//        //STEPS      we keep this to < 255 -- don't need that many
//        numhalfcyclesteps = (uint8_t) (halfcycletime / refreshdelay);
//
//        //STEP SIZE
//        //Given:   ( peakbrightness / halfcycletime ) == ( stepsize / refreshdelay )
//        //        stepsize = refreshdelay ( peakbrightness / halfcycletime)
//        // THE CHALLENGE is that on the extremes our step size is too small.  We can increase this
//        // by gradually ratcheting up the refreshdelay (makes sense for longer pules cycle times).
//        stepsize =   (uint8_t) (((uint16_t)refreshdelay * (uint16_t)peakbrightness ) / halfcycletime );
//        uint8_t x = 1;
//        while( (stepsize < 1) && (x <= 8)) {
//            refreshdelay = refreshdelay *  x;
//            stepsize =   (uint8_t) ((refreshdelay * peakbrightness ) / halfcycletime );
//            x++;
//        }
//        if (stepsize < 1) { stepsize = 1; }
//
//        if(goingup1) {
//            halfcyclenumber++;
//        } else {
//            halfcyclenumber--;
//        }
//        //CHECK FOR DIRECTION FLIP
//        if(halfcyclenumber >= numhalfcyclesteps) {
//            goingup1 = false;
//            halfcyclenumber = numhalfcyclesteps;
//        }
//        if(halfcyclenumber == 0) {
//            goingup1 = true;
//        }
//
//        uint8_t brightness = (uint8_t)(stepsize * halfcyclenumber);
//        color1 = colorWithBrightness( color1, brightness );
//
//        brightness = (uint8_t)(stepsize * (numhalfcyclesteps - halfcyclenumber));
//        color2 = colorWithBrightness( color2, brightness );
//
//
//        //UPDATE ODD NUMBERED PIXELS
//        for(uint16_t i=0; i < NUMPIXELS; i = i+2) {
//            strip.setPixelColor(i, color1);
//        }
//        //UPDATE EVEN NUMBERED PIXELS
//        for(uint16_t i=1; i < NUMPIXELS; i = i+2) {
//            strip.setPixelColor(i, color2);
//        }
//        strip.show();
//        activeDelay(refreshdelay);
//    }
//}
//
//
///**
// *  ---------------- doRainbow() --------------------
// *    cross-fades alternating LEDs on strip
// *    POT1:  Sets Brightness
// *    POT2:  Sets Rainbow Width
// *    POT3:  Sets velocity  (speed / direction)
// **/
//void doRainbow() {
//    uint16_t rawval = 0;
//    uint16_t width;  // = 10;             //From R->G->B->R how wide is our rainbow
//    uint8_t brightness; // = 255;
//    uint16_t delaytime = 50;
//    bool goingright; // = true;
//    uint16_t i = 0;                //pixel index
//    uint16_t r_offset = 0;         //rainbow offset index into our rainbow
//    uint32_t color;                //general purpose color variable
//
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//
//    while(!mode_did_change) {
//        //BRIGHTNESSS -- POT_VALUE_1
//        //      0 = min brightness at peak     255 = max brightness at peak
//        //      ontime = (POT_VALUE_2 / 1023) * cycletime
//        brightness = POT_VALUE_1;
//        //PEAK BRIGHTNESS LOWER LIMIT
//        if (brightness < 10) {
//            brightness = 10;
//        }
//        //WIDTH -- POT_VALUE_2
//        if(POT_VALUE_2 < 128) {
//            width = 1 + (POT_VALUE_2 * NUMPIXELS / 128);   //allow for finer tuning of low range
//        } else {
//            width = NUMPIXELS + (((uint32_t)POT_VALUE_2 - 128) * NUMPIXELS / 32);
//        }
//
//        //SPEED & DIRECTION -- POT 3
//        //     Speed goes from - to +  with delay time longest at midpoint 512
//        //        Max is 517 -- min is 5
//        // -- NO POTS IN PPC   rawval = analogRead(POT_3_PIN);  //10-bit value value from 0 to 1023
//
//        rawval = POT_VALUE_3 * 4;
//
//        if (rawval >= 512) {
//            goingright = true;
//            delaytime = 517 - (rawval - 512);
//        } else {
//            goingright = false;
//            delaytime = 5 +  (rawval * 2);
//        }
//
//        if (goingright) {
//            r_offset++;
//        }  else  {
//            r_offset--;
//        }
////        Serial.print(F("GoingRight / width / delay time / r_offset   "));
////        Serial.print(goingright, DEC);
////        Serial.print(F("  "));
////        Serial.print(width, DEC);
////        Serial.print(F("  "));
////        Serial.print(delaytime, DEC);
////        Serial.print(F("  "));
////        Serial.println(r_offset, DEC);
////        delay(1000);
//
//        for(i=0; i< NUMPIXELS; i++) {
//            color = getColorFrom8BitValue(( r_offset + (i * 256 / width)) & 255);
//            color = colorWithBrightness(color, brightness);
//            strip.setPixelColor(i, color );
//        }
//        strip.show();
//        activeDelay( delaytime );
//
//    }
//}
//
//
///**
// *  ---------------- doCylonSingleColor() --------------------
// *    Colored Cylon scan back and forth on an LED strip
// *    POT1:  Sets Hue
// *    POT2:  Sets Width
// *    POT3:  Sets Velocity (dir, speed)
// **/
//void doCylonSingleColor() {
//    bool inverse = false;
//    uint8_t cylonmode = 0;
//    doCylonWorker(inverse , cylonmode );
//}
//
//
///**
// *  ---------------- doInverseCylonSingleColor() --------------------
// *   Dark (inverse) Cylon scan back and forth on a colored LED strip
// *    POT1:  Sets Hue
// *    POT2:  Sets Width
// *    POT3:  Sets Velocity (dir, speed)
// **/
//void doInverseCylonSingleColor() {
//    bool inverse = true;
//    uint8_t cylonsolid = 0;
//    doCylonWorker( inverse , cylonsolid );
//}
//
///**
// *  ---------------- doCylonRainbow() --------------------
// *   Cylon scan back and forth on a rainbow colored LED strip
// *    POT1:  Sets Rainbow Width
// *    POT2:  Sets Cylon Width
// *    POT3:  Sets Velocity (dir, speed)
// **/
//void doCylonRainbow() {
//    bool inverse = false;
//    uint8_t rainbowmode = 1;
//    doCylonWorker( inverse , rainbowmode );
//}
//
///**
// *  ---------------- doInverseCylonRainbow() --------------------
// *   Dark (inverse) Cylon scan back and forth on a rainbow colored LED strip
// *    POT1:  Sets Rainbow Width
// *    POT2:  Sets Cylon Width
// *    POT3:  Sets Velocity (dir, speed)
// **/
//void doInverseCylonRainbow() {
//    bool inverse = true;
//    uint8_t rainbowmode = 1;
//    doCylonWorker( inverse , rainbowmode );
//}
//
//
///**
// * ---------------- doPlayaFly() --------------------
// *   cross-fades alternating LEDs on strip
// *    POT1:  Hue 1  (Primary Color)
// *    POT2:  Hue 2  (Blip Color)
// *    POT3:  Frequency
// *  set all pixels to primary, fade a random pixel to blip color and back to primary
// *  then delay until next blip
// * Thanks to RRRUs and Tom Corboline for this code.  Originally was
// * called "sparkleColorCycle" and has been reworked for active tweaking
// **/
//// TODO:  POT 3 VALUE SHOULD BE FREQUENCY OF OCCURANCE.
//// TODO;  MODIFY THIS MODE SO THAT THE MODE CAN 'RELEASE' (GLOBAL blip position???)
//void doPlayaFly() {
//    uint32_t primarycolor, blipcolor, color;
//
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//
//    while(!mode_did_change) {
//    	uint16_t blipIndex = random(0, NUMPIXELS);
//
//    	// fade in
//    	// for (uint16_t b=0; b<256; b++) {
//    	// 	primarycolor = getRGBHueFromPot( 1 );
//        //      blipcolor = getRGBHueFromPot( 2 );
//        //      setEntireStripColor( primarycolor);
//        //      color = getBlend(primarycolor, blipcolor, b);
//        //      strip.setPixelColor(blipIndex, color);
//    	//      strip.show();                   //strip.show
//    	// 	if (mode_did_change) { break; }
//    	// 	// activeDelay(2);
//    	// }
//
//    	// fade out blip
//    	for (uint16_t b=255; b>0; b--) {
//            //primarycolor = getRGBHueFromPot( 1 );
//            //blipcolor = getRGBHueFromPot( 2 );
//            primarycolor = getColorFrom8BitValue(POT_VALUE_1);
//            blipcolor    = getColorFrom8BitValue(POT_VALUE_2);
//
//
//            setNumPixelsToColor( primarycolor);
//
//    	    color = getBlend(primarycolor, blipcolor, b);
//            strip.setPixelColor(blipIndex, color);
//    	    strip.show();                   //strip.show
//            activeDelay( 2 );       //activedelay
//    	    if (mode_did_change) { break; }
//    	}
//        //  random delay
//    	unsigned int intervalDelay = random(2,5);
//    	for (uint16_t i=0; i<intervalDelay*10; i++) {
//    	    activeDelay(100);
//            //update the main strip in case user is adjusting
//            //primarycolor = getRGBHueFromPot( 1 );
//            primarycolor = getColorFrom8BitValue(POT_VALUE_1);
//            setNumPixelsToColor( primarycolor );
//            strip.show();
//    	    if (mode_did_change) { break; }
//    	}
//    }
//}
//
//
//
//// ---------------- doStormCloud() --------------------
////    cross-fades alternating LEDs on strip
////    POT1:  Sets Brightness
////    POT2:  Sets Frequency
////    POT3:  Sets  Decay Rate (dim amount)   DELAYTIME might be better?
////Thanks to RRRUs and Tom Corboline for this code.  Originally was
//// called "sparkleColorCycle" and has been reworked for active tweaking
//void doStormCloud() {
//    uint8_t brightness; // = 255;
//    uint32_t color = 0;     //reusable
//    uint8_t  infreq;  // = 2;     // how infrequent do pixels "hit"
//    uint8_t  dimamount; // = 3; // how much do we dim the pixels each pass
//    uint16_t delaytime = 80;  //this tunes with dimamount to set how "full" or empty the strip can be
//    if (NUMPIXELS > 128) {
//        delaytime = 40;
//    }
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//    while(!mode_did_change) {
//        for (int i=0; i<256; i++) {        //cycle through color wheel
//            for (int j=0; j<6; j++) {      //6 possible at each color wheel
//                //BRIGHTNESS - POT_VALUE_1
//                brightness = max(10 , POT_VALUE_1);
//                //INFREQUENCY - POT_VALUE_2
//                infreq = 16 - (POT_VALUE_2 / 16);
//                //DECAY RATE = POT_VALUE_3
//                dimamount = 33 - (POT_VALUE_3 / 8);    //
//
//                color = hsv( i, random(64,162), brightness);
//                if (CURRENT_SEQUENCE == kSEQUENCE_HALLOWEEN) {
//                    //Serial.print(F("Halloween storm cloud colors!"));
//                    //h = 0 : red
//                    // h = 85 : green
//                    // h = 170 : blue
//                    // h = 255 : almost red
//                    if (( i >= 120 ) && (i <= 220 )) {
//                        // avoid cool blues, stick with halloweens
//                        i = random(0,100);
//                    }
//                    color = hsv(i, random(64,162), brightness);
//                }
//
//                // spark a pixel now and then
//                if (random(0, infreq ) == 0) {
//                    uint16_t pixnum = random(0, NUMPIXELS - 1 );
//    		    strip.setPixelColor(pixnum, color);
//    	        }
//
//                //apply decay dimming to entire strip
//                applyDecayDimToEntireStrip(dimamount);
//
//                strip.show();
//    	        activeDelay( delaytime );
//
//                if(mode_did_change) { break; }
//            }
//            if(mode_did_change) { break; }
//        }
//    }
//}
//
//
//
//
//
//// ---------------- doMondrian() --------------------
////    Colorblocks seperated by black pixels.  Mondrian blocking used full saturation fyi
////    POT1:  Sets Brightness
////    POT2:  Sets BlockWidth gaussian center
////    POT3:  Sets Delay Time gaussian center
////Thanks to RRRUs and Tom Corboline for this code.  Originally was
//// called "sparkleColorCycle" and has been reworked for active tweaking
//void doMondrian() {
//    uint8_t brightness; // = 255;
//    uint32_t color = 0;       //reusable
//    uint8_t  blocksize = 2;  // our base block size, others will be multiples of this
//    uint16_t delaytime; // = 3000;
//
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//    //Serial.println(F("INSIDE MONDRIAN"));
//
//    while(!mode_did_change) {
//        brightness = max(10 , POT_VALUE_1);
//        uint16_t blockend = 0;
//        for (uint16_t pixnum=0 ;  pixnum < NUMPIXELS; pixnum++ ) {
//            if ( ( pixnum == blockend ) && ( pixnum != 0 ) ) {         //avoid always setting 0 pixel to black
//                color = npBLACK;
//            }
//            if ( pixnum > blockend) {        //get new block end and color
//                blockend = pixnum + (blocksize * random(1, NUMPIXELS/4));     //so we always have at least 2 colors
//                //              hue            saturation           brightness
//                color = hsv( random(0,255),  random(160, 255),    brightness);
//            }
//            strip.setPixelColor(pixnum, color);
//            if(mode_did_change) { break; }
//        }
//        strip.show();
//        delaytime = 500 +  (100 * random(1, (POT_VALUE_3 / 2)) );    //from 600ms to > 13s
//        //Serial.print(F("Delaytime: "));
//        //Serial.print(delaytime, DEC);
//        activeDelay( delaytime );
//
//        if(mode_did_change) { break; }
//    }
//}
//
//#pragma mark HELPERS & WORKER METHODS
//
//void printPixelPacerSettings() {
//    Serial.print(F("\tLengths: "));Serial.print(LENGTHS, DEC);
//    Serial.print(F("\tPace: ")); Serial.print(PACE, DEC);
//    Serial.print(F("\tRest: "));Serial.print(REST, DEC);
//    Serial.print(F("\tSwimmers: "));Serial.print(SWIMMERS, DEC);
//    Serial.print(F("\tIntervalSpacing: "));Serial.println(INTERVAL, DEC);
//}
//
////
/////**
//// * True up our lead swimmer at key points in time (at the walls)
//// * This is meant to be frequently called by ActiveDelay()
//// */
////void trueUpLeadSwimmer() {
////    if (rtc_datetime_now.unixtime() == dt_this_length_end.unixtime()) {  // compare is matched on the second
////        idx_lead_swimmer = idx_this_length_end;
////        if((idx_this_length_end - idx_lead_swimmer) < NUMPIXELS) {  // guard so we don't increment multiple times
////            // looks like we haven't incremented yet...
////            idx_this_length_end  += NUMPIXELS;     // don't worry, our main loop will set to 0 if we are past swim end
////            dt_this_length_end += PACE;
////        }
////    }
////}
//
////
/////**
//// * Figure out if we are going left to right or right to left. Then
//// * figure out where on the physical pixel strand we end up on.
//// * @param idx_curr_swimmer
//// * @param idx_swim_end
//// * @return
//// */
////uint16_t _getCenterPixel(uint16_t idx_curr_swimmer, uint16_t idx_swim_end) {
////    uint16_t center_pixel = 0;
////    if (idx_curr_swimmer <= idx_swim_end) {
////        //determine curr_center_pixel
////        if ((idx_curr_swimmer / NUMPIXELS) % 2 == 0) {
////            //we are going left to right
////            center_pixel = idx_curr_swimmer % NUMPIXELS;
////        } else {
////            //we are going right to left
////            center_pixel = (NUMPIXELS - 1) - (idx_curr_swimmer % NUMPIXELS);
////        }
////    } else {   // reached end of swim -- leave curr_center_pixel untouched
////        center_pixel = 0;
////    }
////    return center_pixel;
////}
//
//
//// ------------ doCylonWorker() -------- a helper method ----------
//// Cylon Worker method that does the hard work
//// Can be configured to handle the various modes
////
//// @prop inverse   boolean value for cylon inverted or not
//// @prop cylonmode      enum value per below:
//#define CYLON_SOLID_COLOR     0
//#define CYLON_RAINBOW         1
////------------------------------------------------------------------
//void doCylonWorker(bool inverse, uint8_t cylonmode) {
//    uint16_t loopspeed = 3000;     //full round trip time in ms
//    uint32_t delaytime = loopspeed / (NUMPIXELS * 2 );
//    uint32_t c = 0;        //color
//    uint8_t  rainbowoffset = 0;   //index for rainbow modes
//    uint16_t rainbowwidth = NUMPIXELS;
//    uint8_t  dir = 1;       //1 = RIGHT (+)--->   0 = LEFT <----(-)   arbitrary
//    uint16_t leadpos = 0;
//    uint16_t width = 5;     //total width is +/- this number
//    if(mode_did_change) {
//        loadModeSettingsFromEEPROM(CURRENT_MODE);
//        mode_did_change = false;
//        display_needs_update = true;
//    }
//
//    while(!mode_did_change) {
//        //POT_1_PIN
//        if ( cylonmode == CYLON_SOLID_COLOR ) {
//            //SET COLOR - POT_1_PIN
//            //c = getRGBHueFromPot( 1 );
//            c = getColorFrom8BitValue(POT_VALUE_1);
//            if(inverse) {
//                setNumPixelsToColor( c );
//            } else {
//                setNumPixelsToColor( npBLACK);
//            }
//        } else if ( cylonmode == CYLON_RAINBOW ) {
//            //SET rainbowidth - POT_1_PIN
//            if(POT_VALUE_2 < 128) {
//                rainbowwidth = 1 + (POT_VALUE_1 * NUMPIXELS / 128);   //allow for finer tuning of low range
//            } else {
//                rainbowwidth = NUMPIXELS + (((uint32_t)POT_VALUE_1 - 128) * NUMPIXELS / 32);
//            }
//            //Fill in colors if inverted
//            if(inverse) {
//                for(uint16_t i=0; i < NUMPIXELS; i++) {
//                     c = getRainbowPixelColor( rainbowoffset, i, rainbowwidth);
//                     //c = getColorFrom8BitValue(( rainbowoffset + (i * 256 / rainbowwidth)) & 255);
//                     strip.setPixelColor(i, c);
//                }
//            }
//        }
//
//        //SET CYLON WIDTH - POT_2_PIN
//        width = 1 + ((uint32_t)NUMPIXELS * POT_VALUE_2) / 255;     //will max out bit width when NUMPIXELS > 255
//        //SET SPEED - POT_3_PIN
//        // fastest loop speed 8660-8160 = 500.  Slowest = 8660-32
//        loopspeed = 8660 - ( (uint16_t) POT_VALUE_3 * 32 );    // 241 *32 = 7712   948
//        delaytime = (uint32_t) loopspeed / (NUMPIXELS * 2 );    //948/460=2
//
//        if(delaytime < 2) {
//            delaytime = 2;
//        }
//
//
//        //MOVE THE LEAD POSITION
//        if(dir) {  leadpos++;  }
//        else    {  leadpos--;  }
//
//        if (leadpos == 0) {
//            dir = 1;
//        } else if ( leadpos==(NUMPIXELS-1) ) {
//            dir = 0;
//        }
//
//        //MOVE THE RAINBOW OFFSET
//        if ( cylonmode == CYLON_RAINBOW ) {
//            rainbowoffset++;
//            if (rainbowoffset >= 256) {    // gaurd.  should not possible since we're uint8_t
//                rainbowoffset = 0;
//            }
//        }
//
//        // DEVELOPER NOTE:  The cylon has a way of going beyond the NUMPIXELS in
//        // it's simplified calculations.  Rather than put a ton of if/then guards, faster to
//        // mask it all at the very end....
//        // SETTING OUR CYLON BOUNDS
//        if(inverse) {                            //make sure ends are full color of correct color
//            if( cylonmode == CYLON_RAINBOW ) {
//                c = getRainbowPixelColor( rainbowoffset, (leadpos - width - 1), rainbowwidth);
//                //c = getColorFrom8BitValue( (rainbowoffset + ((leadpos - width - 1) * 256 / rainbowwidth)) & 255);
//                strip.setPixelColor(leadpos - (width+1) , c);
//                c = getRainbowPixelColor( rainbowoffset, (leadpos + width + 1), rainbowwidth);
//                //c = getColorFrom8BitValue( (rainbowoffset + ((leadpos + width + 1) * 256 / rainbowwidth)) & 255);
//                strip.setPixelColor(leadpos + (width+1) , c);
//            } else {
//                strip.setPixelColor(leadpos - (width+1) , c);   //make sure ends are full color
//                strip.setPixelColor(leadpos + (width+1) , c);
//            }
//        } else {
//            strip.setPixelColor(leadpos - (width+1) , npBLACK);   //make sure ends are black
//            strip.setPixelColor(leadpos + (width+1) , npBLACK);
//        }
//
//        //SETTING OUR CYLON COLORS
//        for (uint8_t x = 0; x <= width; x++) {
//            uint8_t index = 0;
//            uint8_t brightness; // = 255;
//            if (width <= 255) {
//                index = (255 / width) * x;
//            } else {
//                index = x;
//                //TODO  our max width to 255 for now ##################
//            }
//            if(!inverse) {
//                brightness = (uint8_t) pgm_read_byte_near(  &(kExpoDecay[index]) );
//            } else  {
//                brightness = (uint8_t) pgm_read_byte_near(  &(kExpoDecay[255-index]) );
//            }
//
//            uint32_t dimc;
//            if ( cylonmode == CYLON_RAINBOW ) {
//                c = getRainbowPixelColor( rainbowoffset, (leadpos - x), rainbowwidth);
//                //c = getColorFrom8BitValue(( rainbowoffset + ((leadpos-x) * 256 / rainbowwidth)) & 255);
//            }
//            dimc = colorWithBrightness(c, brightness);
//            strip.setPixelColor(leadpos - x , dimc);
//            if (x != 0) {
//                strip.setPixelColor(leadpos + x , dimc);
//            }
//
//        }
//
//        //MASK OFF ANYTHING THAT IS "OUT OF BOUNDS"
//        for (uint16_t i = NUMPIXELS; i < kMAX_NUMPIXELS; i++) {
//            strip.setPixelColor(i, npBLACK);
//        }
//
//
//        strip.show();
//        activeDelay(delaytime);
//
//   } //end of while loop
//}  // end of doCylonWorker
//
//
//void setNumPixelsToColor(uint32_t c) {
//    for(uint16_t j=0; j < NUMPIXELS; j++) {
//        strip.setPixelColor(j, c);
//    }
//}
//
//
////set all pixels, even those we are not
////actively controlling.  Useful for
////ensuring we are blanking entire possible
////region
//void setMaxNumPixelsToColor(uint32_t c) {
//    for(uint16_t j=0; j < kMAX_NUMPIXELS ; j++) {
//        strip.setPixelColor(j, c);
//    }
//
//}
//
//
////Decay entire strip of pixels.
//void applyDecayDimToEntireStrip( uint8_t dimamount ) {
//    uint32_t color;
//    for(uint16_t i=0; i < NUMPIXELS; i++) {
//        color = strip.getPixelColor(i);
//        color = dimColorWithValue(color, dimamount);
//        strip.setPixelColor(i, color);
//    }
//}
//
//
//uint16_t getVariableRefreshDelayFor(uint16_t cycletime) {
//    long d; // = 50;
//    if (cycletime < 500) {
//       d = 20;            //<25 steps
//    } else if (cycletime < 1000) {
//       d = 25;           //<40 steps
//    } else if (cycletime < 2000) {
//       d = 40;           //<40 steps
//    } else {
//       d = 100;          //<50 steps
//    }
//
//    return (uint16_t)d;
//}
//
//
