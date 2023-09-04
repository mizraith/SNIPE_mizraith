///**
// * Part of SNIPE
// *
// * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
// * See License file in PPC_Main.h
// */
//
//#ifndef _PPC_MODES_H_
//#define _PPC_MODES_H_
//
//#include <Arduino.h>
//#include <avr/pgmspace.h>
//
//
//#ifdef USE_NEOPIXEL_LEDS
//    #include <Adafruit_NeoPixel.h>
//    //#warning "Using Adafruit Neopixel Library"
//#endif
//
//#ifdef USE_WS2801_LEDS
//    //#warning "Using Adafruit WS2801 Library"
//    #include <Adafruit_WS2801.h>
//#endif
//
////#include "PPC_ModeTable.h"
//#include "SNIPE_ExponentialDecay.h"
//// for  getRGBHueFromPot(), colorWithBrightness(), getColorFrom8BitValue(), getBlend(), hsv(), getRainbowPixelColor()
//#include "SNIPE_ColorUtilities.h"
////#include "PPC_PinDefs_and_Constants.h"
////#include "PPC_GLOBALS.h"
////#include "PPC_ColorWipes.h"
////#include "PPC_Displays.h"
////#include "PPC_Settings.h"
////#include "PPC_Interrupts_Timers.h"
//
////-------------------------------------
////  Mode Logic
////-------------------------------------
//void incrementMode( void );
//void decrementMode( void );
//void handlePPCEncoderChange( void );
//void handleNormalModeEncoderChange( void );
//void updateCurrentMode();
//
//
////-------------------------------------
//// Functional Modes
////-------------------------------------
//void doProgrammingMode();
//void doSolidColor();
//void doPulseColor();
//void doStrobeColor();
//void doCrossFade();
//void doRainbow();
//void doCylonSingleColor();
//void doInverseCylonSingleColor();
//void doCylonRainbow();
//void doInverseCylonRainbow();
//void doPlayaFly();
//void doStormCloud();
//void doMondrian();
//void doPixelPacer();
//
////-------------------------------------
//// Mode-specific helpers and workers
////-------------------------------------
//void printPixelPacerSettings();
//void trueUpLeadSwimmer();
//uint16_t _getCenterPixel(uint16_t idx_curr_swimmer, uint16_t idx_swim_end);
//
//#define CYLON_SOLID_COLOR     0
//#define CYLON_RAINBOW         1
//
//void doCylonWorker(bool inverse, uint8_t cylonmode);
//
//void setNumPixelsToColor(uint32_t c);
//
//void setMaxNumPixelsToColor(uint32_t c);
//
//void applyDecayDimToEntireStrip( uint8_t dimamount );
//
//uint16_t getVariableRefreshDelayFor(uint16_t cycletime);
//
//
//
//#endif