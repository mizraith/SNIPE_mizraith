//
// Created by Red Byer on 7/16/23.
//
// To run use: https://www.onlinegdb.com/online_c_compiler
//
//
//////////////////////////////////////////////

/******************************************************************************

/* Online C Compiler and Editor */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// was +1 for null but we added 2 more chars to allow for "0x" so now it's +3.
#define UNS_HEX_STR_SIZE ((sizeof (unsigned)*CHAR_BIT + 3)/4 + 3)
//                         compound literal v--------------------------v
#define U2HS(x) unsigned_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)
// Note 1:  We prepend "0x" to the string.
// Note 2: This strips off leading zeros.
char *unsigned_to_hex_string(unsigned x, char *dest, size_t size) {
    //("0x%X\n", a);
    snprintf(dest, size, "0x%X", x);
    return dest;
}

#define C2HS(x) color_uint_to_hex_string((x), (char[UNS_HEX_STR_SIZE]) {0}, UNS_HEX_STR_SIZE)
// In this cae we return at least 6 digits  0x00FF00.   Could be more if
// the color value is greater than 0xFFFFFF.
//  "0x" is prepended for us.
char *color_uint_to_hex_string(unsigned x, char *dest, size_t size) {
    snprintf(dest, size, "0x%06X", x);
    return dest;
}


int atoui(char *in,unsigned int *out)
{
    char *p;

    for(p = in; *p; p++)
        if (*p > '9' || *p < '0')
            return 1;

    *out = strtoul(in, NULL, 10);
    return 0;
}


int main() {
    // 3 array are formed v               v        v
    printf("Max: %s   0x00FF00:%s  0x12345678: %s\n", U2HS(UINT_MAX), U2HS(0x00FF00), U2HS(0x12345678));
    char *hs = U2HS(rand());
    printf("Random: ");
    puts(hs);
    printf("Color: %s %s %s\n", C2HS(0x00FF00), C2HS(0x0F0F0F), C2HS(0x0000FF));
    // Taking hex string --> int.  From:
    // https://www.techiedelight.com/convert-a-hexadecimal-string-to-an-integer-in-cpp/
    int i;
    sscanf("0F", "%x", &i);   // returns 0 if you give it weird characters.  handles "0x0f" "0f" "0F" all fine
    printf("0F as an int: %d\n", i);

    char str[4][7] = {{"123"},
                      {"-123"},
                      {"wewe"},
                      {"123wew"}};
    unsigned int ui, uc;

    for(uc = 0; uc < 4; uc++) {
        if(atoui(str[uc], &ui)) {
            printf("ERROR: %s\n",str[uc]);
        }
        else{
            printf("OK: %u\n",ui);
        }
    }


    int Z = atoi("55");
    printf("atoui --> %d", Z);


}

/**
* example output from above
Max: 0xFFFFFFFF   0x00FF00:0xFF00  0x12345678: 0x12345678
Random: 0x6B8B4567
Color: 0x00FF00 0x0F0F0F 0x0000FF
*/