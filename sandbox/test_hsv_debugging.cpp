//
// Created by Red Byer on 10/10/23.
//
//
// To run use: https://www.onlinegdb.com/online_c_compiler
//
//
/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
#include <stdint.h>

uint32_t hsv(uint8_t h, uint8_t s, uint8_t v) {
    // h = 0 : red
    // h = 85 : green
    // h = 170 : blue
    // h = 255 : almost red
    uint8_t r, g, b, region, remain, p, q, t;
    uint32_t color;
    //unsigned char region, remainder, p, q, t;

    if (s == 0)  {
        r = v;    //no saturation, everything equals brightness value
        g = v;
        b = v;
    }

    else {
        region = h / 43;
        remain = (h - (region * 43)) * 6;

        p = (v * (255 - s)) >> 8;                             // off by -1 when arduino does it   THE FINAL FIX WAS TO (uint32_t) ALL THE VALUES
        q = (v * (255 - ((s * remain) >> 8))) >> 8;           // off by -1 when arduino does it     THE FINAL FIX WAS TO (uint32_t) ALL THE VALUES
        t = (v * (255 - ((s * (255 - remain)) >> 8))) >> 8;   // off by -1 when arduino does it       THE FINAL FIX WAS TO (uint32_t) ALL THE VALUES

        switch (region)  {
            case 0:
                r = v; g = t; b = p;
                break;
            case 1:
                r = q; g = v; b = p;
                break;
            case 2:
                r = p; g = v; b = t;
                break;
            case 3:
                r = p; g = q; b = v;
                break;
            case 4:
                r = t; g = p; b = v;
                break;
            default:
                r = v; g = p; b = q;
                break;
        }
    }
    printf("\nhue: %3d", h);
    printf("\tregion: %3d ", region);
    printf("\trem: %3d ", remain);
    printf("\tred: %3d ", r);
    printf("\tgreen: %3d ", g);
    printf("\tblue: %3d ", b);
    //color = getColorFromRGB(r, g, b);
    return color;
}

int main()
{
    printf("Hello World\n\n");
    hsv((uint8_t)177, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)176, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)175, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)174, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)173, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)172, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)171, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)170, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)169, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)168, (uint8_t)255, (uint8_t)255);
    hsv((uint8_t)167, (uint8_t)255, (uint8_t)255);
    return 0;
}

// 10/10/23  Arduino r value results seem to be off-by-one when h: 177:172, s=255, v=255
//hue: 177	region: 4	rem: 30	    red: 29	    green: 0	blue: 255
//hue: 176	region: 4	rem: 24	    red: 23	    green: 0	blue: 255
//hue: 175	region: 4	rem: 18	    red: 17	    green: 0	blue: 255
//
//hue: 173	region: 4	rem: 6	    red: 5	    green: 0	blue: 255
//hue: 172	region: 4	rem: 0	    red: 255	green: 0	blue: 255
//hue: 171	region: 3	rem: 252	red: 0	    green: 2	blue: 255
//
//hue: 169	region: 3	rem: 240	red: 0	    green: 14	blue: 255
//hue: 168	region: 3	rem: 234	red: 0	    green: 20	blue: 255




// 10/10/23  -- RESULTS agree with EXCEL calcs.
//hue: 177        region:   4     rem:  30        red:  30        green:   0      blue: 255
//hue: 176        region:   4     rem:  24        red:  24        green:   0      blue: 255
//hue: 175        region:   4     rem:  18        red:  18        green:   0      blue: 255
//hue: 174        region:   4     rem:  12        red:  12        green:   0      blue: 255
//hue: 173        region:   4     rem:   6        red:   6        green:   0      blue: 255
//hue: 172        region:   4     rem:   0        red:   0        green:   0      blue: 255
//hue: 171        region:   3     rem: 252        red:   0        green:   3      blue: 255
//hue: 170        region:   3     rem: 246        red:   0        green:   9      blue: 255
//hue: 169        region:   3     rem: 240        red:   0        green:  15      blue: 255
//hue: 168        region:   3     rem: 234        red:   0        green:  21      blue: 255
//hue: 167        region:   3     rem: 228        red:   0        green:  27      blue: 255