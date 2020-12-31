#pragma once
#include "utils.h"
#include "stream.h"

class H_data{
    public:
        H_data();
        u8 symbol;
};

class H_node {
    public:
        H_node();
        bool    is_terminal ();         // Return wheather it is terminal node
        H_data* terminal_data ();       // Return its terminal data
        void    conv_terminal ();       // Convert to terminal node
        void    triverse (u32 prefix);
        H_node* pos;
        H_node* neg;
};

class Huffman {
    public:
        Huffman (Stream* vs, u8 type);
        i16  get (Stream* vs);  // Get one code word and return run
        void triverse ();       // Triverse whole tree and print
        u8   R;
        i16  S;
        u16  len;
    private:
        void insert (u16 code, u8 len, u8 symbol);
        H_node* root;
        u8  type;
};
