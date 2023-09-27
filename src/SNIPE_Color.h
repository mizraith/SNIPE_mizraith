//
// Created by Red Byer on 9/26/23.
// Flexes the Arduino library for it's String class. Alternatively you can use C++ strings if you
// have a library for your micro that supports them.
//

#ifndef SNIPE_MIZRATIH_SNIPE_COLOR_H
#define SNIPE_MIZRATIH_SNIPE_COLOR_H

#include <Arduino.h>
#include <stdint.h>

class SNIPE_Color {
public:
    const String name {"BLACK"};
    const uint32_t value {0};         // default 0 value



    // dummy constructor for new, just take the defaults
    SNIPE_Color() {
    }
    // paramaterized constructor for direct creation
    SNIPE_Color(const String name,  const uint32_t value ):
    value(value),  // constant member var init syntax
    name(name),
    {
    }

    bool is_equal_value(uint32_t value);
    bool is_equal_name(String name);

};

// These are the strings we will accept commands for
// ALWAYS in upper case
const class SNIPE_Color cRED = SNIPE_Color("RED", 0xFF0000);
const class SNIPE_Color cORANGE = SNIPE_Color("ORANGE", 0xFF5500);
const class SNIPE_Color cYELLOW = SNIPE_Color( "YELLOW", 0xFFFF00);
const class SNIPE_Color cGREEN = SNIPE_Color( "GREEN", 0x00FF00);
const class SNIPE_Color cAQUA = SNIPE_Color( "AQUA", 0x00FFFF);
const class SNIPE_Color cBLUE = SNIPE_Color("BLUE", 0x0000FF);
const class SNIPE_Color cINDIGO = SNIPE_Color("INDIGO" , 0x3300FF);
const class SNIPE_Color cVIOLET = SNIPE_Color("VIOLET", 0xFF00FF);
const class SNIPE_Color cWHITE = SNIPE_Color("WHITE" ,  0xFFFFFF);
const class SNIPE_Color cBLACK = SNIPE_Color("BLACK" ,  0x000000);

const class SNIPE_Color  kCOLORS []   = {
        cRED, cORANGE, cYELLOW, cGREEN, cAQUA, cBLUE, cINDIGO, cVIOLET, cWHITE, cBLACK
};


#endif //SNIPE_MIZRATIH_SNIPE_COLOR_H
