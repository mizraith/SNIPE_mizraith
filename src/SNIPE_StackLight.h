//
// Created by Red Byer on 9/11/23.
//

#ifndef SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H
#define SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H

#include "Adafruit_NeoPixel.h"

#define BLACK 0x000000
#define MODE_DEFAULT 0

class SNIPE_StackLight {
public:                         // Access specifier
    uint8_t lightpin;                // PIN to use... I think it's a uint.
    uint8_t neopixelmask;     //  e.g.  NEO_GRB + NEO_KHZ800
    uint32_t color {BLACK};         // default 0 value
    uint32_t current_color {BLACK}; // currently working color
    uint8_t mode {MODE_DEFAULT};    // solid state
    uint16_t cycle_ms {500};        // 500 ms full cycle time
    unsigned long update_time {0};       // immediate
    bool flash_is_on {false};
    bool pulse_going_up {true};
    uint8_t numpixels {8};         // how many pixes in this stack light
    uint8_t perc_lit {100};         // single digit, no floats please
    Adafruit_NeoPixel *strip;
    bool mode_did_change {false};

    // dummy constructor for new, just take the defaults
    SNIPE_StackLight() {
    }
    // paramaterized constructor for direct creation
    SNIPE_StackLight(uint8_t lightpin, uint16_t numpixels, uint8_t neopixel_mask)
    {
            this->lightpin = lightpin;
            this->numpixels = numpixels;
            this->neopixelmask = neopixel_mask;
    }

    void change_mode(uint8_t new_mode);
    void update_pulse_color();
    void update_flash_color();
    void setup_strip();
};


#endif //SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H
