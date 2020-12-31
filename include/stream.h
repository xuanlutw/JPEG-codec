#pragma once
#include "utils.h"

class Stream {
    public:
        Stream (char* file_name);
        ~Stream ();
        u32  read (u8 len);                 // Read at most 32 bits from buffer
        u8   read_u8 ();
        u16  read_u16 ();
        void put (u32 content, u8 len);     // Put some bits back into buffer
        u8   next_marker ();                // Go to next start code and return it       
        void set_skip_fl (bool fl);         // Set the flag of skip 0xFFFF
    private:
        FILE* fp;
        u32   buf;
        u8    buf_len;
        bool  skip_fl;
        u32   counter;
        u8    marker;
        bool  read_buf ();
        void  align ();
};
