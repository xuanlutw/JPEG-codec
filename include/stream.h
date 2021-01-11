#pragma once
#include "utils.h"

class Stream {
    public:
        Stream (char* file_name, u8 type);
        ~Stream ();
        u32  read (u8 len);                 // Read at most 32 bits from buffer
        u8   read_u8 ();
        u16  read_u16 ();
        void write (u32 content, u8 len);
        void write_u8 (u8 content);
        void write_u16 (u16 content);
        void write_marker (u8 marker);
        void put (u32 content, u8 len);     // Put some bits back into buffer
        u8   next_marker ();                // Go to next start code and return it       
        void set_skip_fl (bool fl);         // Set the flag of skip 0xFFFF
        u32  get_size ();
    private:
        FILE* fp;
        u8    type;
        u32   buf;
        u8    buf_len;
        bool  skip_fl;
        u32   counter;
        u8    marker;
        bool  read_buf ();
        void  read_align ();
        bool  write_buf ();
        void  write_align ();
};
