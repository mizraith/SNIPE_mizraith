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
    const uint8_t lightpin {5};  // PIN to use D5 default... I think it's a uint.
    const uint8_t neopixel_type {NEO_GRB + NEO_KHZ800};     //  e.g.  NEO_GRB + NEO_KHZ800
    uint32_t color {BLACK};         // default 0 value
    uint32_t current_color {BLACK}; // currently working color
    uint8_t mode {MODE_DEFAULT};    // solid state
    uint16_t cycle_ms {500};        // 500 ms full cycle time
    unsigned long update_time {0};       // immediate
    bool flash_is_on {false};
    bool pulse_going_up {true};
    const uint8_t numpixels {24};         // how many pixels in this stack light
    uint8_t perc_lit {100};         // single digit, no floats please
    Adafruit_NeoPixel *strip;
    bool mode_did_change {false};

    // dummy constructor for new, just take the defaults
    SNIPE_StackLight() {
    }
    // paramaterized constructor for direct creation
    SNIPE_StackLight( uint8_t lightpin, const uint8_t numpixels, uint8_t neopixel_mask):
    lightpin(lightpin),
    neopixel_type(neopixel_mask),
    numpixels(numpixels)  // constant member var init syntax
    {
    }

    void change_mode(uint8_t new_mode);
    void update_pulse_color();
    void update_flash_color();
    void setup_strip();
    void print_info();
};


#endif //SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H
