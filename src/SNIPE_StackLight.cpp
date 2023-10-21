//
// Created by Red Byer on 9/11/23.
//

#include "SNIPE_StackLight.h"
#include "SNIPE_ColorUtilities.h"

extern const uint8_t SLA_PIN;

void SNIPE_StackLight::change_mode(uint8_t new_mode) {
    // At this point, new_mode is already vetted against the appropriate values.
    // handle mode switches.  Where we want to keep the color on even when flash/pulse has changed
    if (new_mode != mode) {
        mode_did_change = true;
        mode = new_mode;
        update_time = millis();

        // handle steady-state alarm modes
        if (alarm_enabled) {
            if (mode == MODE_OFF) {
                alarm_enabled = false;
                digitalWrite(SLA_PIN, LOW);
            } else if ( (mode == MODE_STEADY) or (mode == MODE_RAINBOW) ) {
                digitalWrite(SLA_PIN, HIGH);
            }  // WE WILL HANDLE THE REST IN handle_strobing_alarm()
        }


    }
    return;
}

/**
 * 1) Calculate number active
 * 2) Handle Next Strobe Step and/or mode change
 * 3) Handle Next Flash Step  and/or mode change
 */
void SNIPE_StackLight::update() {
    uint8_t numactive;
    numactive = int((float)this->numpixels * ((float)this->perc_lit / 100));
    numactive = min(numactive, this->numpixels);   // make sure our math doesn't overshoot.
    if (this->mode == MODE_OFF) {
        this->current_color = BLACK;
        if (this->mode_did_change) {
            this->mode_did_change = false;
            this->flash_is_on = false;
        }
    }
    if (this->mode == MODE_STEADY) {
        this->current_color = this->color;
        if (this->mode_did_change) {
            this->mode_did_change = false;
            this->flash_is_on = false;
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

    this->handle_strobing_alarm();

    // At this point, current_color holds the calculated value.
    // Do the actual work
    for(uint16_t i=0; i < numactive; i++) {
        this->strip->setPixelColor(i, this->current_color);
    }
    for(uint16_t i=numactive; i < this->numpixels; i++) {
        this->strip->setPixelColor(i, BLACK);
    }
    this->strip->show();
}


void SNIPE_StackLight::update_pulse_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
        pulse_going_up = false;
        current_color = color;
    } else if (millis() < update_time) {    // not a flip, how much longer until we do?
        // how far along are we in our cycle_ms
        unsigned long remaining_ms = cycle_ms - (update_time - millis());
        //when going down, or when remaining == cycle_ms, we start at 255
        uint8_t brightness = (uint8_t)((255 * remaining_ms) / (cycle_ms / 2));  // our cycleposition within our half step is our brightness
        if (pulse_going_up) {
            brightness = 255 - brightness;  // going up, start from black
        }
        //Serial.print("rem: ");Serial.print(remaining_ms);Serial.print("\tbrt: ");Serial.println(brightness);
        current_color = colorWithBrightnessExpo(color, brightness);
        return;

    } else if (millis() > update_time) {    // flip
        pulse_going_up = !pulse_going_up;
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

void SNIPE_StackLight::update_rainbow_color() {
    // calculate and put result in current color
    if (mode_did_change) {
        mode_did_change = false;
        pulse_going_up = false;
        current_color = color;
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
        pulse_going_up = !pulse_going_up;  // why not
        update_time = millis() + (unsigned long) (cycle_ms);
    }
    return;
}

void SNIPE_StackLight::handle_strobing_alarm() {
    if (!alarm_enabled) {
        if (digitalRead(SLA_PIN)) {
            digitalWrite(SLA_PIN, LOW);
        }
        return;
    } else {   // alarm is enabled
        if (mode == MODE_FLASH) {
            if (flash_is_on) {
                digitalWrite(SLA_PIN, HIGH);
            } else {
                digitalWrite(SLA_PIN, LOW);
            }
        }
        if (mode == MODE_PULSE) {
            if (pulse_going_up) {
                digitalWrite(SLA_PIN, HIGH);
            } else {
                digitalWrite(SLA_PIN, LOW);
            }
        }
    }
}



void SNIPE_StackLight::setup_strip() {
    this->strip = new Adafruit_NeoPixel(this->numpixels, this->lightpin, this->neopixel_type);
    this->strip->begin();
    this->strip->setBrightness(255);
    this->strip->show();
}

void SNIPE_StackLight::print_info() {
    Serial.println(F("#"));
    Serial.print(F("#---------- SLINFO: [ "));Serial.print(this->id);Serial.println(F(" ] -----------"));
    Serial.print(F("#\t&this:     "));Serial.println((unsigned int)this, DEC);
    Serial.print(F("#\tsize_of:   "));Serial.println(sizeof(SNIPE_StackLight));
    Serial.print(F("#\tcolor:     "));Serial.println(this->color);
    Serial.print(F("#\tmode:      "));Serial.println(this->mode);
    Serial.print(F("#\tcycle_ms:  "));Serial.println(this->cycle_ms);
    Serial.print(F("#\tflash on:  "));Serial.println(this->flash_is_on);
    Serial.print(F("#\tpulse->up: "));Serial.println(this->pulse_going_up);
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
//    bool flash_is_on{false};
//    bool pulse_going_up{true};
//    uint8_t numpixels{8};         // how many pixes in this stack light
//    uint8_t perc_lit{100};         // single digit, no floats please
//    Adafruit_NeoPixel strip;
//    bool mode_did_change{false};
}
