#include <cstdio>
#include <cmath>
#include "utils.h"

u8 zz[64] = { 0,  1,  5,  6, 14, 15, 27, 28, \
              2,  4,  7, 13, 16, 26, 29, 42, \
              3,  8, 12, 17, 25, 30, 41, 43, \
              9, 11, 18, 24, 31, 40, 44, 53, \
             10, 19, 23, 32, 39, 45, 52, 54, \
             20, 22, 33, 38, 46, 51, 55, 60, \
             21, 34, 37, 47, 50, 56, 59, 61, \
             35, 36, 48, 49, 57, 58, 62, 63 };

u8 bandpass (double val) {
    if (val >= 255.0)
        return 255;
    else if (val <= 0.0)
        return 0;
    else
        return (u8) round(val);
}

void write_u32 (FILE* fp, u32 num) {
    // Write little-endian u32

    for (u8 i = 0; i < 4; ++i) {
        fprintf(fp, "%c", num & 0xFF);
        num >>= 8;
    }
    return;
}

void write_u16 (FILE* fp, u16 num) {
    // Write little-endin u16

    for (u8 i = 0; i < 2; ++i) {
        fprintf(fp, "%c", num & 0xFF);
        num >>= 8;
    }
    return;
}

