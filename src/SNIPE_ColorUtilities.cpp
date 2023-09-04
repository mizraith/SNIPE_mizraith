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



#pragma mark Color Math Functions
/**
 *          Add two numbers, and calculate the modulo
 *          of the sum and a third number, M.
 *          In other words, it returns (A+B) % M.
 *          It is designed as a compact mechanism for
 *          incrementing a 'mode' switch and wrapping
 *          around back to 'mode 0' when the switch
 *          goes past the end of the available range.
 *          e.g. if you have seven modes, this switches
 *          to the next one and wraps around if needed:
 *            mode = addmod8( mode, 1, 7);
 *      See 'mod8' for notes on performance.
 *      Thanks to FastLED library under MIT License.
 **/
uint8_t addmod8( uint8_t a, uint8_t b, uint8_t m) {
    //#if defined(__AVR__)
    asm volatile (
            "       add %[a],%[b]    \n\t"
            "L_%=:  sub %[a],%[m]    \n\t"
            "       brcc L_%=        \n\t"
            "       add %[a],%[m]    \n\t"
            : [a] "+r" (a)
    : [b] "r"  (b), [m] "r" (m)
    );

    //straight C implementation
    //a += b;
    //    while( a >= m) a -= m;
    //#endif
    return a;
}



/**
 * Quick add one byte to another, saturating at 0xFF
 * Thanks to FastLED library, under MIT License.
 * @param i - first byte to add
 * @param j - second byte to add
 * @returns the sum of i & j, capped at 0xFF
**/
uint8_t qadd8( uint8_t i, uint8_t j) {
// straight C implementation follows
//    unsigned int t = i + j;
//    if( t > 255) t = 255;
//    return t;

    asm volatile(
        /* First, add j to i, conditioning the C flag */
            "add %0, %1    \n\t"

            /* Now test the C flag.
              If C is clear, we branch around a load of 0xFF into i.
              If C is set, we go ahead and load 0xFF into i.
            */
            "brcc L_%=     \n\t"
            "ldi %0, 0xFF  \n\t"
            "L_%=: "
            : "+a" (i)
            : "a"  (j)
            );

    return i;
}

/**
 * Quick subtract one byte from another, saturating at 0x00
 *  Thanks to FastLED library, under MIT License.
 *  @param i - first byte to subtract from
 *  @param j - second byte to subtract with
 * @returns i - j with a floor of 0
 **/
uint8_t qsub8( uint8_t i, uint8_t j) {
// straight C implementation
//int t = i - j;
//    if( t < 0) t = 0;
//    return t;

    asm volatile(
        /* First, subtract j from i, conditioning the C flag */
            "sub %0, %1    \n\t"

            /* Now test the C flag.
             If C is clear, we branch around a load of 0x00 into i.
             If C is set, we go ahead and load 0x00 into i.
             */
            "brcc L_%=     \n\t"
            "ldi %0, 0x00  \n\t"
            "L_%=: "
            : "+a" (i)
            : "a"  (j)
            );

    return i;
}


/**
 *  Quick calculate an integer average of two unsigned
 *      8-bit integer values (uint8_t).
 *      Fractional results are rounded down, e.g. avg8(20,41) = 30
 *      Thanks to FastLED library, under MIT License.
 **/
uint8_t qavg8( uint8_t i, uint8_t j) {
// Straight C implementation
//  return (i + j) >> 1;

    asm volatile(
        /* First, add j to i, 9th bit overflows into C flag */
            "add %0, %1    \n\t"
            /* Divide by two, moving C flag into high 8th bit */
            "ror %0        \n\t"
            : "+a" (i)
            : "a"  (j)
            );

    return i;
}





/**
 * Get abs() of a signed 8-bit uint8_t
 * Thanks to FastLED library under MIT License
 */
int8_t qabs8( int8_t i)  {
// straight C implementation
//if( i < 0) i = -i;
//    return i;

    asm volatile(
        /* First, check the high bit, and prepare to skip if it's clear */
            "sbrc %0, 7 \n"

            /* Negate the value */
            "neg %0     \n"

            : "+r" (i) : "r" (i)
            );
    return i;
}

//8-bit absolute value function -- check
//the webs for an explanation, but it has
//to do with 2's complement negative notation
int8_t getAbs(int8_t n) {
    int8_t const mask = n >> 7;
    return ((n ^ mask) - mask);
}





