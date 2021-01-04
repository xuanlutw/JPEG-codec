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

void Header::write (Stream* os) {
    os->write_marker(this->type);
    os->write_u16(this->len + 2);
    for (u8 i = 0; i < this->len; ++i)
        os->write_u8(this->data[i]);
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

void DHT::write (Stream* os) {
    u8  c;
    u16 len = 2;
    os->write_marker(M_DHT);
    for (u8 i = 0; i < N_DHT; ++i) {
        if (this->dc[i])
            len += 17 + this->dc[i]->len;
        if (this->ac[i])
            len += 17 + this->ac[i]->len;
    }
    os->write_u16(len);
    for (u8 i = 0; i < N_DHT; ++i) {
        if (this->dc[i]) {
            os->write_u8(i + (TYPE_DC << 4));
            this->dc[i]->write_opt(os);
        }
        if (this->ac[i]) {
            os->write_u8(i + (TYPE_AC << 4));
            this->ac[i]->write_opt(os);
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
