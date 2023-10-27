//
// Created by Red Byer on 9/11/23.
//

#include "SNIPE_StackLight.h"
#include "SNIPE_ColorUtilities.h"

extern void prioritize_serial(uint8_t);

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

void SNIPE_StackLight::set_color(uint32_t new_color) {
    color = new_color;
    current_color = new_color;
    strip_changed = true;
    return;
}

uint32_t SNIPE_StackLight::get_color() {
    return color;
}

void SNIPE_StackLight::set_percentage(uint8_t percent){
    if (percent > 100) {
        percent = 100;
    }
    perc_lit = percent;
    strip_changed = true;
}

uint8_t SNIPE_StackLight::get_percentage() {
    return perc_lit;
}

/**
 * 1) Calculate number active
 * 2) Handle Next Strobe Step and/or mode change
 * 3) Handle Next Flash Step  and/or mode change
 * 4) Only sends strip.show() if something has changed or it has been a long time since a refresh
 * Should be safe to call frequently (every 25-50ms) regardless of mode.
 */
void SNIPE_StackLight::update() {
    uint8_t numactive;

    if (mode_did_change) {
        // mode_did_change = false;  // this flip handled by each mode
        flash_on_pulse_up = false;   // flash mode does this different
        current_color = color;       // off mode does this different
    }

    //  THE MODES UPDATE current_color
    if (this->mode == MODE_OFF) {
        this->current_color = BLACK;
        if (this->mode_did_change) {
            this->mode_did_change = false;
        }
    }
    if (this->mode == MODE_STEADY) {
        this->current_color = this->color;
        if (this->mode_did_change) {
            this->mode_did_change = false;
        }
    }
    if (this->mode == MODE_FLASH) {
        this->update_flash_color();
    }
    if (this->mode == MODE_PULSE) {
        this->update_pulse_color();
    }
    if (this->mode == MODE_RAINBOW) {
        this->update_rainbow_color();
    }

    // At this point, current_color holds the calculated value.
    // Do the actual work
    numactive = int((float)this->numpixels * ((float)this->perc_lit / 100));
    numactive = min(numactive, this->numpixels);   // make sure our math doesn't overshoot.
    for(uint16_t i=0; i < numactive; i++) {
        this->strip->setPixelColor(i, this->current_color);
    }
    for(uint16_t i=numactive; i < this->numpixels; i++) {
        this->strip->setPixelColor(i, BLACK);
    }

    //  SEE IF current_color is different
    if (current_color != last_color) {
        strip_changed = true;
        last_color = current_color;
    }

    //  Has it been a LONG time since we've updated?
    if ((millis() - last_updated) > 5000) {
        Serial.print(F("Force_updating_strip "));Serial.println(id, DEC);
        strip_changed = true;
    }

    // ONLY UPDATE if something has changed, like current_color or percentage.
    if (strip_changed) {
        prioritize_serial(id);
        strip->show();    // moved this out to here for better control
        strip_changed = false;
        last_updated = millis();
    }
}

/**
 * figure out if it is time to update current_color
 */
void SNIPE_StackLight::update_pulse_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
    } else if (millis() < update_time) {    // not a flip, how much longer until we do?
        // how far along are we in our cycle_ms
        unsigned long remaining_ms = cycle_ms - (update_time - millis());
        //when going down, or when remaining == cycle_ms, we start at 255
        uint8_t brightness = (uint8_t)((255 * remaining_ms) / (cycle_ms / 2));  // our cycleposition within our half step is our brightness
        if (flash_on_pulse_up) {
            brightness = 255 - brightness;  // going up, start from black
        }
        //Serial.print("rem: ");Serial.print(remaining_ms);Serial.print("\tbrt: ");Serial.println(brightness);
        current_color = colorWithBrightnessExpo(color, brightness);
        return;

    } else if (millis() > update_time) {    // flip
        flash_on_pulse_up = !flash_on_pulse_up;
        update_time = millis() + (unsigned long) (cycle_ms / 2);
    }
    return;
}

/**
 * figure out if it is time to update current_color in a strobe like flash
 */
void SNIPE_StackLight::update_flash_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
        flash_on_pulse_up = true;
    } else if (millis() > update_time) {    // change state
        if (flash_on_pulse_up) {
            flash_on_pulse_up = false;
            current_color = BLACK;
        } else {
            flash_on_pulse_up = true;
            current_color = color;
        }
    } else if (millis() < update_time) {    // nothing has changed.  current_color is still valid.
        return;
    }
    update_time = millis() + (unsigned long) (cycle_ms / 2);  // changed mode or state
    return;
}

/**
 * figure out if it is time to update current_color in a rainbow wash
 */
void SNIPE_StackLight::update_rainbow_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
    } else if (millis() < update_time) {    // not a flip, how much longer until we do?
        // how far along are we in our cycle_ms. Our hue maps from 0 --> cycle_ms : 0 -> 255
        unsigned long remaining_ms = cycle_ms - (update_time - millis());
        // Going from red->blue so we want to start at 0
        uint8_t hue = 255 - (uint8_t)((255 * remaining_ms) / (cycle_ms));  // our cycleposition within our half step is our brightness
        current_color = hsv(hue, 255, 255);  // no saturation, full bright.
//        uint8_t r = getRedFromColor(current_color);
//        uint8_t g = getGreenFromColor(current_color);
//        uint8_t b = getBlueFromColor(current_color);
//        Serial.print("rem:\t");Serial.print(remaining_ms);Serial.print("\thue:\t");Serial.print(hue);
//        Serial.print("\tr:\t");Serial.print(r);Serial.print("\tg:\t");Serial.print(g);Serial.print("\tb:\t");Serial.println(b);
        return;

    } else {    // flip
        update_time = millis() + (unsigned long) (cycle_ms);
    }
    return;
}


void SNIPE_StackLight::setup_strip() {
    strip = new Adafruit_NeoPixel(numpixels, lightpin, neopixel_type);
    //Adafruit_NeoPixel strip(numpixels, lightpin, neopixel_type);
    strip->begin();
    strip->clear();
    strip->setBrightness(255);
    prioritize_serial(id);  // ALWAYS CALL BEFORE CALLING SHOW...MAKE SURE WE GET OUR MESSAGE FIRST
    strip->show();
}

void SNIPE_StackLight::print_info() {
    Serial.println(F("#"));
    Serial.print(F("#---------- SLINFO: [ "));Serial.print(this->id);Serial.println(F(" ] -----------"));
    Serial.print(F("#\t&this:     "));Serial.println((unsigned int)this, DEC);
    Serial.print(F("#\tsize_of:   "));Serial.println(sizeof(SNIPE_StackLight));
    Serial.print(F("#\tcolor:     "));Serial.println(this->color, HEX);
    Serial.print(F("#\tmode:      "));Serial.println(this->mode);
    Serial.print(F("#\tcycle_ms:  "));Serial.println(this->cycle_ms);
    Serial.print(F("#\tflash on:  "));Serial.println(this->flash_on_pulse_up);
    Serial.print(F("#\tnumpixels: "));Serial.println(this->numpixels);
    Serial.print(F("#\tperc_lit:  "));Serial.println(this->perc_lit);
    Serial.print(F("#\t&(*strip): "));Serial.println((unsigned int)&(this->strip), DEC);
    Serial.print(F("#\t&strip: "));Serial.println((unsigned int)this->strip, DEC);

    char buffstr[kCOLORLENGTH];
    const char * string_in_progmem = (const char *) this->colorname_p;
    strcpy_P(buffstr, string_in_progmem);
    Serial.print(F("#\tcolorname: "));Serial.println(buffstr);
    Serial.println(F("#------------------------------------"));

    //Serial.print(F("\tstripbytes: "));Serial.println((uint16_t )this->strip->numBytes);


    //    uint32_t color{BLACK};         // default 0 value
//    uint32_t current_color{BLACK}; // currently working color
//    uint8_t mode{MODE_DEFAULT};    // solid state
//    uint16_t cycle_ms{500};        // 500 ms full cycle time
//    unsigned long update_time{0};       // immediate
//    bool flash_on_pulse_up{false};
//    bool pulse_going_up{true};
//    uint8_t numpixels{8};         // how many pixes in this stack light
//    uint8_t perc_lit{100};         // single digit, no floats please
//    Adafruit_NeoPixel strip;
//    bool mode_did_change{false};
}
