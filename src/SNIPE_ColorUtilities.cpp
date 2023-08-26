/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */

#include "SNIPE_ColorUtilities.h"
#include <Arduino.h>    // we make good use of String() class
#include <avr/pgmspace.h>

//*******************************************************************************************
//*******************************************************************************************
//
//     COLOR UTILITY METHODS
//
//
//*******************************************************************************************
//*******************************************************************************************

///**
// * Used frequently to get our input color from a a given pot
// * @param potnumber 1 2 or 3
// * @return
// */
//uint32_t getRGBHueFromPot(uint8_t potnumber) {
//    uint16_t rawval;
//    if (potnumber == 1) {
//        rawval = analogRead(POT_1_PIN);
//    } else if (potnumber == 2) {
//        rawval = analogRead(POT_2_PIN);
//    } else if (potnumber == 3) {
//        rawval = analogRead(POT_3_PIN);
//    } else {
//        rawval = 0;
//    }
//
//    uint32_t color = getColorFrom10BitValue(rawval);
//    return color;
//}


uint8_t getRedFromColor(uint32_t color) {
    return (uint8_t)(color >> kSHIFT_RED);   
}
uint8_t getGreenFromColor(uint32_t color) {
    return (uint8_t)(color >> kSHIFT_GREEN);   
}
uint8_t getBlueFromColor(uint32_t color) {
    return (uint8_t)(color >> kSHIFT_BLUE);   
}


//     Input a value 0 to 255 to get a color value.
//     The colors are a transition r - g - b - back to r.
uint32_t getColorFrom8BitValue(uint8_t WheelPos) {
    if(WheelPos < 85) {
        return getColorFromRGB(255 - WheelPos * 3, WheelPos * 3, 0);     //was 2nd
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return getColorFromRGB(0 , 255 - WheelPos * 3, WheelPos * 3);    //was 3rd
    } else {
        WheelPos -= 170;
        return getColorFromRGB( WheelPos *3, 0, 255 - WheelPos * 3);     //was 1st
    }
}


//Given a 10-bit value, return the color.  This is much like getColorFrom8Bit(below)
uint32_t getColorFrom10BitValue(uint16_t rawval) {
    uint16_t region, primary, secondary;
    uint8_t r,g,b;
    // --- region 0 ---- | --- region 1 ---- | ---- region 2 ---- |
    // 0 = R  170 = RG  341 = G  511 = GB  682 = B   852 = BR   1023 = almost R
    region = rawval / 341;
    primary =  1023 - ( (rawval - ( region * 341)) * 3);
    secondary = (rawval - ( region * 341)) * 3;
    
    switch (region) {
        case 0:
            //assign numbers over.  Right shift 2 to convert from 10-bit val to 8-bit
            r = primary >> 2; g = secondary >> 2; b = 0;
            break;
        case 1:
            r = 0; g = primary >> 2; b = secondary >> 2;
            break;
        case 2:
            r = secondary >> 2; g = 0; b = primary >> 2;
            break; 
        default:
            r = primary >> 2, g = secondary >> 2; b = 0;
            break; 
    }

    uint32_t color = getColorFromRGB(r, g, b);
    return color;
}

//utility function for building our RGB 32bit value
// THIS FUNCTION IS IDENTICAL TO THE getColorFromRGB 
// but takes 16-bit numbers in a desperate attempt to
// get past the "Even register number required" compiler issue
uint32_t getColorFrom16bitRGB(uint16_t r, uint16_t g, uint16_t b) {
    uint32_t color = 0; 
    color = color + (((uint32_t) r ) << kSHIFT_RED);
    color = color + (((uint32_t) g ) << kSHIFT_GREEN);
    color = color + (((uint32_t) b ) << kSHIFT_BLUE);
    return color;
}



//utility function for building our RGB 32bit value
uint32_t getColorFromRGB(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = 0; 
  
    //Shift the bits into BRG order
    color = color + (((uint32_t) r ) << kSHIFT_RED);
    color = color + (((uint32_t) g ) << kSHIFT_GREEN);
    color = color + (((uint32_t) b ) << kSHIFT_BLUE);
//    Serial.print(color, HEX);
//    Serial.println();
//    delay(500);
    return color;
}



//dim the color in factors of 2....basically right shift
// the r, g, b bytes.   Can only shift 0-7 
uint32_t dimColorWithShift(uint32_t c, uint8_t shift) {
    uint8_t r,g,b;
    r = getRedFromColor(c);
    g = getGreenFromColor(c);
    b = getBlueFromColor(c);
    if (shift > 7) {
        return c;    //do nothing
    }
    r = r >> shift;
    g = g >> shift;
    b = b >> shift;
  
    c = getColorFromRGB(r, g, b);
  
    return c;
}




uint32_t dimColorWithValue(uint32_t c, uint8_t dim) {
    uint8_t r,g,b;
    r = getRedFromColor(c);
    g = getGreenFromColor(c);
    b = getBlueFromColor(c);
      
    if (r >= dim) {
        r -= dim;
    } else {
        r = 0;
    }
    if (g >= dim) {
        g -= dim;
    } else {
        g = 0;
    }
    if (b >= dim) {
        b -= dim;
    } else {
        b = 0;
    }

    c = getColorFromRGB(r, g, b);
    return c;
}



//dim the color proportional to dimvalue/255
// the r, g, b bytes.  
uint32_t colorWithBrightness(uint32_t c, uint8_t brightness) {
    uint16_t brite = (uint16_t)brightness;
    uint8_t r,g,b;
    r = getRedFromColor(c);
    g = getGreenFromColor(c);
    b = getBlueFromColor(c);
  
    uint32_t temp = 0;
    temp = brite * r;              //we have to add this extra line or we get a gcc compiler error
    r = (uint8_t) (temp / 256);
  
    temp = brite * g;
    g = (uint8_t) (temp / 256);
   
    temp = brite * b;
    b = (uint8_t) (temp / 256);

    c = getColorFromRGB(r, g, b);
    return c;
}


//Thanks again to RRRus and Tom Corboline for this method
//  Returns a 32-bit RGB value based on HSV inputs.
//  Modified as standalone method.
uint32_t hsv(uint8_t h, uint8_t s, uint8_t v) {
    // h = 0 : red
    // h = 85 : green
    // h = 170 : blue
    // h = 255 : almost red
    uint8_t r, g, b;
    uint32_t color;
    unsigned char region, remainder, p, q, t;

    if (s == 0)  {
        r = v;    //no saturation, everything equals brightness value
	g = v;
        b = v;
    }
    
    else {
        region = h / 43;
        remainder = (h - (region * 43)) * 6; 

	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * remainder) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

	switch (region)  {
	    case 0:
		r = v; g = t; b = p;
		break;
	    case 1:
		r = q; g = v; b = p;
		break;
	    case 2:
		r = p; g = v; b = t;
		break;
	    case 3:
		r = p; g = q; b = v;
		break;
	    case 4:
		r = t; g = p; b = v;
		break;
	    default:
		r = v; g = p; b = q;
		break;
        }
    }
    color = getColorFromRGB(r, g, b);
    return color;
}

/**
 * Saturating no frills add of two rgb colors.
 * @param c1
 * @param c2
 * @return
 */
uint32_t addColors(uint32_t c1, uint32_t c2) {
    uint8_t r1 = getRedFromColor(c1);
    uint8_t g1 = getGreenFromColor(c1);
    uint8_t b1 = getBlueFromColor(c1);
    uint8_t r2 = getRedFromColor(c2);
    uint8_t g2 = getGreenFromColor(c2);
    uint8_t b2 = getBlueFromColor(c2);

    r1 = qadd8(r1, r2);
    g1 = qadd8(g1, g2);
    b1 = qadd8(b1, b2);

    c1 = getColorFrom16bitRGB(r1, g1, b1);

    return c1;
}


//Blend 2 colors.  Interp is how strong c1 is compared to c2 and should be <= 255
// An interp value of 255 makes c2 100%, while a value of 0 makes c1 100%.
uint32_t getBlend(uint32_t c1, uint32_t c2, uint16_t interp) {
    uint16_t interpi = 255-interp;
    //because of gcc error (even register number required), had to break the lines up into multiple operations
    //Actual formula is   (blendedcolor = color*interpi + (othercolor * interp) ) / 256
    uint8_t r1 = getRedFromColor(c1);
    uint8_t g1 = getGreenFromColor(c1);
    uint8_t b1 = getBlueFromColor(c1);
    uint16_t r2 = (uint16_t)r1 * interpi;    //color * interpi
    uint16_t g2 = (uint16_t)g1 * interpi;
    uint16_t b2 = (uint16_t)b1 * interpi;
    
    r1 = getRedFromColor(c2);
    g1 = getGreenFromColor(c2);
    b1 = getBlueFromColor(c2);
    r2 +=  (uint16_t)r1 * interp;           //othercolor * interp
    g2 +=  (uint16_t)g1 * interp;
    b2 +=  (uint16_t)b1 * interp;
      
    r2 = r2 >> 8;                          // divide by 256
    g2 = g2 >> 8;
    b2 = b2 >> 8;
    //use special method to get past even register number compiler issue
    c1 = getColorFrom16bitRGB((uint8_t)r2, (uint8_t)g2, (uint8_t)b2);
    return c1;
}



uint32_t getRainbowPixelColor(uint8_t rainbowoffset, uint16_t pixelindex, uint16_t rainbowwidth) {
    return getColorFrom8BitValue( (rainbowoffset + (pixelindex * 256 / rainbowwidth)) & 255);
}  




//Useful debugging function
void serialPrintColorValue(uint32_t c) {
    uint8_t r,g,b;
    r = getRedFromColor(c);
    g = getGreenFromColor(c);
    b = getBlueFromColor(c);
    Serial.print(F("R G B:  "));
    Serial.print(r, DEC);
    Serial.print(F("  "));
    Serial.print(g, DEC);
    Serial.print(F("  "));
    Serial.println(b, DEC);
}







