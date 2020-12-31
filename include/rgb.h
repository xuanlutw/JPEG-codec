#pragma once
#include "utils.h"
#include "picture.h"

class RGB {
    public:
        RGB (Picture* pic);
        void dump_ppm (char* filename);
        void dump_bmp (char* filename);
        u16  v_size;
        u16  h_size;
        u8*  R;
        u8*  G;
        u8*  B;
};
