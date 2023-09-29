//
// Created by Red Byer on 9/26/23.
// Flexes the Arduino library for it's String class. Alternatively you can use C++ strings if you
// have a library for your micro that supports them.
//

#ifndef SNIPE_COLOR_H
#define SNIPE_COLOR_H

#include <stdint.h>
#include <Arduino.h>
#include "SNIPE_Strings.h"

class SNIPE_Color {
public:
    const char * name_p;          // PROGMEM STRING
    const uint32_t value;         // default 0 value

    // paramaterized constructor for direct creation
    SNIPE_Color(const char * name_ptr,  const uint32_t value ):
    name_p(name_ptr), // constant member var init syntax.
    value(value)
    {
    };

    bool is_equal_value(uint32_t value) const;
    bool is_equal_name(const char * name) const;

};

extern const class SNIPE_Color cRED;
extern const class SNIPE_Color cORANGE;
extern const class SNIPE_Color cYELLOW;
extern const class SNIPE_Color cGREEN;
extern const class SNIPE_Color cAQUA;
extern const class SNIPE_Color cBLUE;
extern const class SNIPE_Color cINDIGO;
extern const class SNIPE_Color cVIOLET;
extern const class SNIPE_Color cWHITE;
extern const class SNIPE_Color cBLACK;

extern const class SNIPE_Color *kCOLORS [];

extern const size_t kCOLORS_len;

#endif //SNIPE_MIZRAITH_SNIPE_COLOR_H
