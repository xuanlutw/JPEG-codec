#pragma once
#include "utils.h"
#include "stream.h"
#include "header.h"

class Picture {
    public:
        Picture (DHT* H, DQT* Q);
        void read_SOF0 (Stream* vs);
        void read_DRI  (Stream* vs);
        void read_SOS  (Stream* vs);
        void decode    (Stream* vs);
        void trans     (Stream* vs, Stream* os);
        void dump_ppm  (char* ppm_name);
        DHT* H;
        DQT* Q;

        // SOF0
        u8  prec;
        u16 v_size;
        u16 h_size;
        u8  n_color;
        u8  h_fact[N_CHAN];
        u8  v_fact[N_CHAN];
        u8  q_id[N_CHAN];

        // DRI
        u16 restart_interval;

        // SOS
        u8  ac_id[N_CHAN];
        u8  dc_id[N_CHAN];

        // Other
        u16  v_size_ext;
        u16  h_size_ext;
        u8   mcu_h_size;
        u8   mcu_v_size;
        i16* pix[N_CHAN];
};
