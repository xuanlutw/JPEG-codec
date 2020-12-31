#include <cstdlib>
#include <cstdio>
#include "huffman.h"

H_data::H_data () {
    this->symbol  = 0;
}

H_node::H_node () {
    this->pos = NULL;
    this->neg = NULL;
}

bool H_node::is_terminal () {
    return (this == this->pos);
}

H_data* H_node::terminal_data () {
    check (this->is_terminal(), "Not a terminal node!");
    return (H_data*)this->neg;
}

void H_node::conv_terminal () {
    check (!this->pos && !this->neg, "Can't convert to a terminal node!");
    this->pos = this;
    this->neg = (H_node*)(new H_data());
}

void H_node::triverse (u32 prefix) {
    if (this->is_terminal())
        printf("%04x\t%d\n", prefix,    \
                this->terminal_data()->symbol);
    else {
        prefix <<= 1;
        if (this->neg)
            this->neg->triverse(prefix);
        else
            printf("%04x NEG-HAIYAA\n", prefix);
        prefix++;
        if (this->pos)
            this->pos->triverse(prefix);
        else
            printf("%04x POS-HAIYAA\n", prefix);
    }
}

Huffman::Huffman (Stream* vs, u8 type) {
    this->len  = 0;
    this->root = new H_node();
    this->type = type;

    u8 num_symbol[N_HDEP];
    for (u8 code_len = 0; code_len < N_HDEP; ++code_len) {
        num_symbol[code_len] = vs->read_u8();
        this->len += num_symbol[code_len];
    }

    u16 code = 0;
    u8  symbol;
    for (u8 code_len = 0; code_len < N_HDEP; ++code_len) {
        for (u8 i = 0; i < num_symbol[code_len]; ++i) {
            symbol = vs->read_u8();
            this->insert(code, code_len + 1, symbol);
            ++code;
        }
        code <<= 1;
    }
}

void Huffman::insert (u16 code, u8 code_len, u8 symbol) {
    H_node* now = this->root;
    for (; code_len > 0; --code_len)
        if ((code >> (code_len - 1)) & 1)
            now = now->pos = now->pos? now->pos: new H_node();
        else
            now = now->neg = now->neg? now->neg: new H_node();
    now->conv_terminal();
    now->terminal_data()->symbol = symbol;
}

i16 Huffman::get (Stream* vs) {
    H_node* node_now = this->root;
    while (!node_now->is_terminal()) {
        if (vs->read(1)) {
            //printf("1");
            check(node_now = node_now->pos, "Wrong Huffman code");
        }
        else {
            //printf("0");
            check(node_now = node_now->neg, "Wrong Huffman code");
        }
    }
    //printf("\n");
    
    u8 S_len = node_now->terminal_data()->symbol & 0x0f;
    this->R  = node_now->terminal_data()->symbol >> 4;
    this->S  = vs->read(S_len);
    if (S_len && !(this->S >> (S_len - 1)))
        this->S = -(this->S ^ ((1 << S_len) - 1));
    return this->S;
}

void Huffman::triverse () {
    printf("Code\tSymbol\n");
    this->root->triverse(0);
}
