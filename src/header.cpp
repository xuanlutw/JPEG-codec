#include <cstdlib>
#include <cstdio>
#include "header.h"

Header::Header (Stream* vs, u8 type) {
    this->type = type;
    this->len  = vs->read_u16() - 2;
    this->data = new u8[this->len];
    for (u8 i = 0; i < this->len; ++i)
        this->data[i] = vs->read_u8();
}

Header::~Header () {
    delete this->data;
}

DHT::DHT () {
    for (u8 j = 0; j < N_DHT; ++j) {
        this->dc[j] = NULL;
        this->ac[j] = NULL;
    }
}

void DHT::read (Stream* vs) {
    i16 len = vs->read_u16() - 2;
    u8  c;
    while (len) {
        c = vs->read_u8();
        switch (c >> 4) {
            case TYPE_DC:
                this->dc[c & 0x0f] = new Huffman(vs, c >> 4);
                len -= 17 + this->dc[c & 0x0f]->len;
                break;
            case TYPE_AC:
                this->ac[c & 0x0f] = new Huffman(vs, c >> 4);
                len -= 17 + this->ac[c & 0x0f]->len;
                break;
        }
    }
}

DQT::DQT () {
}

void DQT::read (Stream* vs) {
    i16 len = vs->read_u16() - 2;
    u8  c;
    while (len) {
        c = vs->read_u8();
        this->prec[c & 0x0f] = c >> 4;
        for (int i = 0; i < 64; ++i)
            if (this->prec[c & 0x0f])
                this->q[c & 0x0f][i] = vs->read_u16();
            else
                this->q[c & 0x0f][i] = vs->read_u8();
        len = len - 65;
    }
}
