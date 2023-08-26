/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */

#ifndef _SNIPE_ColorUtilities_H_
#define _SNIPE_ColorUtilities_H_

#include <Arduino.h>
#include "SNIPE_GeneralUtilities.h"

//Bit shift operations use the following.  RGB or GRB impact these.
#define kSHIFT_RED    16
#define kSHIFT_GREEN  8
#define kSHIFT_BLUE   0


//*******************************************************************************************
//*******************************************************************************************
//                            COLOR DEFINITIONS
//
//  prepend name with "np" to avoid conflicting
//  with OLED defines
//  assuming 32bit per color RGB
//*******************************************************************************************
//*******************************************************************************************
//extern const uint32_t  npRED;
//extern const uint32_t  npORANGE;
//extern const uint32_t  npYELLOW;
//extern const uint32_t  npGREEN;
//extern const uint32_t  npAQUA;
//extern const uint32_t  npBLUE;
//extern const uint32_t  npINDIGO;
//extern const uint32_t  npVIOLET;
//extern const uint32_t  npWHITE;
//extern const uint32_t  npBLACK;

//npBLACK#define npRED     (uint32_t)0x00FF0000
//#define npORANGE  (uint32_t)0x00FF5500
//#define npYELLOW  (uint32_t)0x00FFFF00
//#define npGREEN   (uint32_t)0x0000FF00
//#define npAQUA    (uint32_t)0x0000FFFF
//#define npBLUE    (uint32_t)0x000000FF
//#define npINDIGO  (uint32_t)0x003300FF
//#define npVIOLET  (uint32_t)0x00FF00FF
//#define npWHITE   (uint32_t)0x00FFFFFF
//#define npWHITE2  (uint32_t)0x00777777
//#define npBLACK   (uint32_t)0x00000000

//uint32_t getRGBHueFromPot(uint8_t potnumber);    // not using pots in PPC

uint8_t getRedFromColor(uint32_t color);
uint8_t getGreenFromColor(uint32_t color);
uint8_t getBlueFromColor(uint32_t color);

uint32_t getColorFrom8BitValue(uint8_t WheelPos);
uint32_t getColorFrom10BitValue(uint16_t rawval);
uint32_t getColorFrom16bitRGB(uint16_t r, uint16_t g, uint16_t b);
uint32_t getColorFromRGB(uint8_t r, uint8_t g, uint8_t b);

uint32_t dimColorWithShift(uint32_t c, uint8_t shift);
uint32_t dimColorWithValue(uint32_t c, uint8_t dim);

uint32_t colorWithBrightness(uint32_t c, uint8_t brightness);

uint32_t hsv(uint8_t h, uint8_t s, uint8_t v);

uint32_t addColors(uint32_t c1, uint32_t c2);
uint32_t getBlend(uint32_t c1, uint32_t c2, uint16_t interp);

uint32_t getRainbowPixelColor(uint8_t rainbowoffset, uint16_t pixelindex, uint16_t rainbowwidth);

void serialPrintColorValue(uint32_t c);



#endif
