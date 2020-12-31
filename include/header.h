#pragma once
#include "utils.h"
#include "stream.h"
#include "huffman.h"

class Header {
    public:
        Header (Stream* vs, u8 type);
        ~Header ();
        u16 type;
        u16 len;
        u8* data;
};

class DHT {
    public:
        DHT ();
        void read (Stream* vs);
        Huffman* dc[N_DHT];
        Huffman* ac[N_DHT];
};

class DQT {
    public:
        DQT ();
        void read (Stream* vs);
        u8  prec[2];
        u16 q[2][64];
};
