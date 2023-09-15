//
// Created by Red Byer on 9/11/23.
//

#include "SNIPE_StackLight.h"


//
//public:                         // Access specifier
//    uint32_t color{BLACK};         // default 0 value
//    uint32_t current_color{BLACK}; // currently working color
//    uint8_t mode{MODE_DEFAULT};    // solid state
//    uint16_t cycle_ms{500};        // 500 ms full cycle time
//    unsigned long update_time{0};       // immediate
//    bool flash_is_on{false};
//    bool pulse_going_up{true};
//    uint8_t numpixels{8};         // how many pixes in this stack light
//    uint8_t perc_lit{100};         // single digit, no floats please
//    Adafruit_NeoPixel strip;
//    bool mode_did_change{false};
//

void SNIPE_StackLight::change_mode(uint8_t new_mode) {
    // At this point, new_mode is already vetted against the appropriate values.
    // handle mode switches.  Where we want to keep the color on even when flash/pulse has changed
    if (new_mode != mode) {
        mode_did_change = true;
        mode = new_mode;
        update_time = millis();
    }
    return;
}

void SNIPE_StackLight::update_pulse_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
        pulse_going_up = false;
        current_color = color;
    } else if (millis() < update_time) {    // not a flip, how much longer until we do?
        // TODO: calculate a dim step
        // TODO: set current color
        return;
    } else if (millis() > update_time) {    // flip
        pulse_going_up = !pulse_going_up;
        if (pulse_going_up) {
            current_color = BLACK;
        } else {
            current_color = color;
        }
        update_time = millis() + (unsigned long) (cycle_ms / 2);
    }
    return;
}

void SNIPE_StackLight::update_flash_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
        flash_is_on = true;
        current_color = color;
    } else if (millis() > update_time) {    // change state
        if (flash_is_on) {
            flash_is_on = false;
            current_color = BLACK;
        } else {
            flash_is_on = true;
            current_color = color;
        }
    } else if (millis() < update_time) {    // nothing has changed.  current_color is still valid.
        return;
    }
    update_time = millis() + (unsigned long) (cycle_ms / 2);  // changed mode or state
    return;
}


void SNIPE_StackLight::setup_strip() {
    this->strip = new Adafruit_NeoPixel(this->numpixels, this->lightpin, this->neopixelmask);
}

void SNIPE_StackLight::print_info() {
    Serial.println(F("----------- StackLight Info ----------"));
    Serial.print(F("\t&this:     "));Serial.println((unsigned int)this, DEC);
    Serial.print(F("\tsize_of:   "));Serial.println(sizeof(SNIPE_StackLight));
    Serial.print(F("\tcolor:     "));Serial.println(this->color);
    Serial.print(F("\tmode:      "));Serial.println(this->mode);
    Serial.print(F("\tcycle_ms:  "));Serial.println(this->cycle_ms);
    Serial.print(F("\tflash on:  "));Serial.println(this->flash_is_on);
    Serial.print(F("\tpulse->up: "));Serial.println(this->pulse_going_up);
    Serial.print(F("\tnumpixels: "));Serial.println(this->numpixels);
    Serial.print(F("\tperc_lit:  "));Serial.println(this->perc_lit);
    Serial.print(F("\t&(*strip): "));Serial.println((unsigned int)&(this->strip), DEC);
    Serial.print(F("\t&strip: "));Serial.println((unsigned int)this->strip, DEC);
    //Serial.print(F("\tstripbytes: "));Serial.println((uint16_t )this->strip->numBytes);


    //    uint32_t color{BLACK};         // default 0 value
//    uint32_t current_color{BLACK}; // currently working color
//    uint8_t mode{MODE_DEFAULT};    // solid state
//    uint16_t cycle_ms{500};        // 500 ms full cycle time
//    unsigned long update_time{0};       // immediate
//    bool flash_is_on{false};
//    bool pulse_going_up{true};
//    uint8_t numpixels{8};         // how many pixes in this stack light
//    uint8_t perc_lit{100};         // single digit, no floats please
//    Adafruit_NeoPixel strip;
//    bool mode_did_change{false};
}
