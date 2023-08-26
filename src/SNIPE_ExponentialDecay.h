/**
 * Part of SNIPE
 *
 * WAS PART OF mizraith_NeopixelController, branched on 2/23/2016
 * See License file in PPC_Main.h
 */
#ifndef _SNIPE_ExponentialDecay_H_
#define _SNIPE_ExponentialDecay_H_

#include <avr/pgmspace.h>
// linear input, exponential output to map to LED log output response
//In the development folder there is an excel file with this sequence.
//Here's an exponential decay function,from x = 0:255   z = ROUND(255 / (1.025 ^x))
static const uint8_t kExpoDecay[] PROGMEM = { 255,249,243,237,231,225,220,215,209,204,199,194,190,185,180,176,
                                            172,168,163,160,156,152,148,145,141,138,134,131,128,125,122,119,
                                            116,113,110,107,105,102,100, 97, 95, 93, 90, 88, 86, 84, 82, 80,
                                            78,76,74,72,71,69,67,66,64,62,61,59,58,57,55,54,
                                            53,51,50,49,48,46,45,44,43,42,41,40,39,38,37,36,
                                            35,35,34,33,32,31,30,30,29,28,28,27,26,26,25,24,
                                            24,23,23,22,22,21,21,20,20,19,19,18,18,17,17,16,
                                            16,16,15,15,15,14,14,14,13,13,13,12,12,12,11,11,
                                            11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7,
                                            7,7,7,7,7,6,6,6,6,6,6,6,5,5,5,5,
                                            5,5,5,5,4,4,4,4,4,4,4,4,4,4,3,3,
                                            3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,
                                            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
                                            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                            1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 };

#endif