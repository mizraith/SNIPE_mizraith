//
// Created by Red Byer on 9/26/23.
//

#include "SNIPE_Color.h"

// These are the strings we will accept commands for
// ALWAYS in upper case
const class SNIPE_Color cRED =    SNIPE_Color(str_RED,     0xFF0000);
const class SNIPE_Color cORANGE = SNIPE_Color(str_ORANGE,  0xFF5500);
const class SNIPE_Color cYELLOW = SNIPE_Color( str_YELLOW, 0xFFFF00);
const class SNIPE_Color cGREEN =  SNIPE_Color( str_GREEN,  0x00FF00);
const class SNIPE_Color cAQUA =   SNIPE_Color( str_AQUA,   0x00FFFF);
const class SNIPE_Color cBLUE =   SNIPE_Color(str_BLUE,    0x0000FF);
const class SNIPE_Color cINDIGO = SNIPE_Color(str_INDIGO , 0x3300FF);
const class SNIPE_Color cVIOLET = SNIPE_Color(str_VIOLET,  0xFF00FF);
const class SNIPE_Color cWHITE =  SNIPE_Color(str_WHITE,   0xFFFFFF);
const class SNIPE_Color cBLACK =  SNIPE_Color(str_BLACK,   0x000000);

const class SNIPE_Color *  kCOLORS []  = {
        &cRED,
        &cORANGE,
        &cYELLOW,
        &cGREEN,
        &cAQUA,
        &cBLUE,
        &cINDIGO,
        &cVIOLET,
        &cWHITE,
        &cBLACK
};

const size_t kCOLORS_len = sizeof(kCOLORS) / sizeof(kCOLORS[0]);


bool SNIPE_Color::is_equal_value(uint32_t colorvalue) const {
    if (this->value == colorvalue) {
        return true;
    }
    else {
        return false;
    }
}


bool SNIPE_Color::is_equal_name(const char * name) const{
    // TODO:  convert to uppercase.   For now, user MUST PROVIDE THE ALL CAPS string

    if (strcmp_P(name, this->name_p) == 0) {
        return true;
    }
    else {
        return false;
    }

}