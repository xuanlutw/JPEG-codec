#pragma once
#include "utils.h"
#include "stream.h"

class H_data{
    public:
        H_data();
        u8  symbol;
        u16 opt_code;
        u16 opt_len;
};

class H_node {
    public:
        H_node();
        bool    is_terminal ();         // Return wheather it is terminal node
        H_data* terminal_data ();       // Return its terminal data
        void    conv_terminal ();       // Convert to terminal node
        void    triverse (u32 prefix);
        H_node* insert (H_node* to_insert);
        void    comp_depth ();
        //void    comp_depth (u8 val);
        H_node* sort_by_depth (u16 len);
        H_node* pos;
        H_node* neg;
        H_node* next;
        H_node* next_leaf;
        u32     counter;
        u8      depth;
};

class Huffman {
    public:
        Huffman (Stream* vs, u8 type);
        i16  get (Stream* vs);  // Get one code word and return run
        void trans (Stream* vs, Stream* os);
        void triverse ();       // Triverse whole tree and print
        void optimal ();
        void write_opt (Stream* os);
        u8   R;
        i16  S;
        u16  len;
    private:
        void insert (u16 code, u8 len, u8 symbol);
        H_node* root;
        H_node* leafs;
        u8   type;
};
