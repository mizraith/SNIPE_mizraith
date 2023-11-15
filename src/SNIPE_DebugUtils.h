//
// Created and improved on by Red Byer on 11/15/23 for SNIPE v4.    github.com/mizraith/SNIPE
// Special thanks to Arduino Forum for this nugget:  https://forum.arduino.cc/t/serial-debug-macro/64259/3
//

#ifndef SNIPE_DEBUGUTILS_H
#define SNIPE_DEBUGUTILS_H

/**
 * define DEBUG in your code as the main on/off switch.
 * define DETAIL_DEBUG_ENABLED to switch debug messages into much mroe detail.
 */

#ifdef DEBUG
    #ifdef DETAIL_DEBUG_ENABLED
        #define DEBUG_PRINT(...)                     \
                Serial.print(__VA_ARGS__)
        #define DEBUG_PRINTLN(...)                 \
                Serial.print(F("# "));              \
                Serial.print(millis());             \
                Serial.print(F(":"));              \
                Serial.print(__PRETTY_FUNCTION__); \
                Serial.print(F(":"));              \
                Serial.print(__LINE__);            \
                Serial.print(F(": "));              \
                Serial.println(__VA_ARGS__)
    #else
        #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
        #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #endif
#else
        #define DEBUG_PRINT(...)
        #define DEBUG_PRINTLN(...)
#endif

#endif // SNIPE_DEBUGUTILS_H