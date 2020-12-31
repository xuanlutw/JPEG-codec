#pragma once
#include "utils.h"
#include "stream.h"
#include "huffman.h"

class Block {
    public:
        Block (Stream* vs, Huffman* DC, Huffman* AC, u16* q, i16* dc_pred);
        void inverse_scan ();
        void inverse_q (u16* q);
        void inverse_DCT ();        // 2D IDCT
        void inverse_DCT1 ();       // 1D IDCT
        void inverse_DCT1_fast ();  // Implement B.G.Lee's IDCT algo.
        void print ();
        i16 data[64];
};

