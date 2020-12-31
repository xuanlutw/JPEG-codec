#include <cstdlib>
#include <cstdio>
#include "picture.h"
#include "block.h"

Picture::Picture (DHT* H, DQT* Q) {
    this->H = H;
    this->Q = Q;
}

void Picture::read_SOF0 (Stream* vs) {
    u8  id;
    u16 len          = vs->read_u16();
    this->prec       = vs->read_u8();
    this->v_size     = vs->read_u16();
    this->h_size     = vs->read_u16();
    this->n_color    = vs->read_u8();
    this->mcu_h_size = 0;
    this->mcu_v_size = 0;
    check(this->n_color == 3, "Only support YCbCr mode!");
    for (u8 i = 0; i < this->n_color; ++i) {
        id = vs->read_u8();
        this->h_fact[id] = vs->read_u8();
        this->v_fact[id] = this->h_fact[id] & 0x0f;
        this->h_fact[id] = this->h_fact[id] >> 4;
        this->q_id[id]   = vs->read_u8();
        if (this->v_fact[id] > this->mcu_v_size)
            this->mcu_v_size = this->v_fact[id];
        if (this->h_fact[id] > this->mcu_h_size)
            this->mcu_h_size = this->h_fact[id];
    }
}

void Picture::read_DRI (Stream* vs) {
    u16 len                = vs->read_u16();
    this->restart_interval = vs->read_u16();
    check(this->restart_interval == 0, "Support no DRI");
}

void Picture::read_SOS (Stream* vs) {
    u8  id;
    u16 len       = vs->read_u16();
    this->n_color = vs->read_u8();
    check(this->n_color == 3, "Only support YCbCr mode!");
    for (u8 i = 0; i < this->n_color; ++i) {
        id = vs->read_u8();
        this->ac_id[id] = vs->read_u8();
        this->dc_id[id] = this->ac_id[id] >> 4;
        this->ac_id[id] = this->ac_id[id] & 0x0f;
    }

    // Check magic number
    check(vs->read_u8() == 0x00, "Wrong SOS-1\n");
    check(vs->read_u8() == 0x3F, "Wrong SOS-2\n");
    check(vs->read_u8() == 0x00, "Wrong SOS-3\n");
}

void Picture::decode (Stream* vs) {
    this->v_size_ext = (this->v_size + 8 * this->mcu_v_size - 1) / (8 * this->mcu_v_size) * (8 * this->mcu_v_size);
    this->h_size_ext = (this->h_size + 8 * this->mcu_h_size - 1) / (8 * this->mcu_h_size) * (8 * this->mcu_h_size);

    // Only Support 420
    this->pix[1] = new i16[this->v_size_ext * this->h_size_ext]();
    this->pix[2] = new i16[this->v_size_ext * this->h_size_ext]();
    this->pix[3] = new i16[this->v_size_ext * this->h_size_ext]();
    i16 dc_pred[N_CHAN] = {0};
    vs->set_skip_fl(FL_SKIP);
    // For each mcu
    for (u32 my = 0; my < this->v_size_ext / this->mcu_v_size / 8; ++my)
        for (u32 mx = 0; mx < this->h_size_ext / this->mcu_h_size / 8; ++mx)
            // For each channel
            for (u8 chan = 1; chan < 4; ++chan)
                // For each block
                for (u8 by = 0; by < this->v_fact[chan]; ++by)
                    for (u8 bx = 0; bx < this->h_fact[chan]; ++bx) {
                        Block block(vs, this->H->dc[this->dc_id[chan]], \
                                        this->H->ac[this->ac_id[chan]], \
                                        this->Q->q[this->q_id[chan]],    \
                                        dc_pred + chan);
                        // Copy
                        for (u8 y = 0; y < 8; ++y)
                            for (u8 x = 0; x < 8; ++x)
                                this->pix[chan][idx2((my * this->v_fact[chan] + by) * 8 + y, \
                                                     (mx * this->h_fact[chan] + bx) * 8 + x, \
                                                     this->h_size_ext * this->h_fact[chan] / this->mcu_h_size)] \
                                    = block.data[idx(y, x)];
                    }
    vs->set_skip_fl(FL_NSKIP);
}
