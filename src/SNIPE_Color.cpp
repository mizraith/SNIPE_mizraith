//
// Created by Red Byer on 9/26/23.
//

#include "SNIPE_Color.h"



bool SNIPE_Color::is_equal_value(uint32_t value){
    if (this->value == value) {
        return true;
    }
    else {
        return false;
    }
}



bool SNIPE_Color::is_equal_name(String name) {
    name.toUpperCase();

    if (name.equals(this->name)) {
        return true;
    }
    else {
        return false;
    }

}