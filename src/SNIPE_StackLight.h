//
// Created by Red Byer on 9/11/23.
//

#ifndef SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H
#define SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H

#include "Adafruit_NeoPixel.h"
#include "SNIPE_Strings.h"

// used in several places as a default
#define BLACK 0x000000

#pragma mark Stack Light Variables
//default mode is solid
#define MODE_DEFAULT 0
#define MODE_OFF     0
#define MODE_STEADY  1
#define MODE_FLASH   2
#define MODE_PULSE   3
#define MODE_RAINBOW 4
//#define MODE_CYLON_RING  3



class SNIPE_StackLight {
public:                         // Access specifier
    const uint8_t id;
    const uint8_t lightpin {5};  // PIN to use D5 default... I think it's a uint.
    const uint8_t neopixel_type {NEO_GRB + NEO_KHZ800};     //  e.g.  NEO_GRB + NEO_KHZ800
    Adafruit_NeoPixel *strip;
    char * colorname_p;   // pointer to string in progmem
    bool strip_changed {true};
    // dummy constructor for new, just take the defaults
    SNIPE_StackLight(): id(1) {
    }
    //  constructor function
    SNIPE_StackLight(uint8_t id, uint8_t lightpin, uint8_t neopixel_mask):
    id(id),
    lightpin(lightpin),
    neopixel_type(neopixel_mask)
    {
        this->colorname_p = (char *) str_EMPTY;
    }

    uint8_t get_numpixels();
    void set_numpixels(uint8_t numpixels, bool firsttime);
    void set_numpixels(uint8_t);
    void set_mode(uint8_t new_mode);
    uint8_t get_mode();
    void set_brightness(uint8_t brightness);
    uint8_t get_brightness();
    void set_color(uint32_t new_color);
    uint32_t get_color();
    void set_percentage(uint8_t percent);
    uint8_t get_percentage();
    void set_cycle_ms(uint16_t cycle_ms);
    uint16_t get_cycle_ms();
    bool get_flash_on();
    void update();
    void update_pulse_color();
    void update_flash_color();
    void update_rainbow_color();
    void setup_strip();
    void print_info();


private:
    // colors and percentages are private so that we can monitor and set flag on updates
    uint8_t numpixels {16};         // how many pixels in this stack light
    uint8_t mode {MODE_DEFAULT};    // solid state
    bool mode_did_change {false};
    uint8_t brightness {255};
    uint8_t last_brightness {255};
    uint32_t color {BLACK};         // our base color...what we would flash or pulse
    uint32_t current_color {BLACK}; // currently working color
    uint32_t last_color {BLACK}; // currently working color
    uint8_t perc_lit {100};         // single digit, no floats please
    uint16_t cycle_ms {1000};        // 500 ms full cycle time
    bool flash_on_pulse_up {false};
    unsigned long update_time {0};       // immediate
    unsigned long last_updated {0};      // make sure we re-send info every so often
};


#endif //SNIPE_MIZRATIH_SNIPE_STACKLIGHT_H
