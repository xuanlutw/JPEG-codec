#include <cstdlib>
#include <cstdio>
#include "huffman.h"

H_data::H_data () {
    this->symbol   = 0;
    this->opt_code = 0;
    this->opt_len  = 0;
}

H_node::H_node () {
    this->pos       = NULL;
    this->neg       = NULL;
    this->next      = NULL;
    this->next_leaf = NULL;
    this->counter   = 0;
    this->depth     = 0;
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
        printf("%04x\t%d\t%d\t%d\n", prefix,   \
                this->terminal_data()->symbol, \
                this->depth,                   \
                this->counter);
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

H_node* H_node::insert (H_node* to_insert) {
    if (this->counter > to_insert->counter) {
        to_insert->next = this;
        return to_insert;
    }
    else if (this->next == NULL) {
        to_insert->next = NULL;
        this->next      = to_insert;
        return this;
    }
    else {
        this->next = this->next->insert(to_insert);
        return this;
    }
}

void H_node::comp_depth () {
    if (this->is_terminal()) {
        ++this->depth;
        return;
    }
    if (this->neg)
        this->neg->comp_depth();
    if (this->pos)
        this->pos->comp_depth();
}

//void H_node::comp_depth (u8 val) {
    //this->depth = val;
    //if (this->is_terminal())
        //return;
    //if (this->neg)
        //this->neg->comp_depth(val + 1);
    //if (this->pos)
        //this->pos->comp_depth(val + 1);
//}

H_node* H_node::sort_by_depth (u16 len) {
    if (len == 1)
        return this;

    H_node* fst_part = this;
    H_node* snd_part = this;
    H_node* tmp      = this;
    for (u16 i = 0; i < (len >> 1) - 1; ++i)
        tmp = tmp->next_leaf;
    snd_part       = tmp->next_leaf;
    tmp->next_leaf = NULL;

    fst_part = fst_part->sort_by_depth(len >> 1);
    snd_part = snd_part->sort_by_depth(len - (len >> 1));

    H_node* merge_head = NULL;
    H_node* merge_tail = NULL;
    while (true) {
        if (!merge_head) {
            if (fst_part->depth < snd_part->depth) {
                merge_head = merge_tail = fst_part;
                fst_part   = fst_part->next_leaf;
            }
            else {
                merge_head = merge_tail = snd_part;
                snd_part   = snd_part->next_leaf;
            }
        }
        else if (!fst_part) {
            merge_tail->next_leaf = snd_part;
            break;
        }
        else if (!snd_part) {
            merge_tail->next_leaf = fst_part;
            break;
        }
        else if (fst_part->depth < snd_part->depth) {
            merge_tail->next_leaf = fst_part;
            fst_part              = fst_part->next_leaf;
            merge_tail            = merge_tail->next_leaf;
        }
        else {
            merge_tail->next_leaf = snd_part;
            snd_part              = snd_part->next_leaf;
            merge_tail            = merge_tail->next_leaf;
        }
    }
    return merge_head;
}

Huffman::Huffman (Stream* vs, u8 type) {
    this->len   = 0;
    this->root  = new H_node();
    this->leafs = NULL;
    this->type  = type;

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
    now->next_leaf = this->leafs;
    this->leafs = now;
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
    
    ++node_now->counter;
    u8 S_len = node_now->terminal_data()->symbol & 0x0f;
    this->R  = node_now->terminal_data()->symbol >> 4;
    this->S  = vs->read(S_len);
    if (S_len && !(this->S >> (S_len - 1)))
        this->S = -(this->S ^ ((1 << S_len) - 1));
    return this->S;
}

void Huffman::trans (Stream* vs, Stream* os) {
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
    os->write(node_now->terminal_data()->opt_code, node_now->terminal_data()->opt_len);
    os->write(this->S, S_len);
    if (S_len && !(this->S >> (S_len - 1)))
        this->S = -(this->S ^ ((1 << S_len) - 1));
}

void Huffman::triverse () {
    printf("Code\tSymbol\n");
    this->root->triverse(0);
}

void Huffman::optimal () {
    // Remove zero counter symbol
    while (this->leafs && !this->leafs->counter) {
        this->leafs = this->leafs->next_leaf;
        --this->len;
    }
    for (H_node* node_now = this->leafs; node_now; node_now = node_now->next_leaf)
        while (node_now->next_leaf && !node_now->next_leaf->counter) {
            node_now->next_leaf = node_now->next_leaf->next_leaf;
            --this->len;
        }
    // Add unused symbol
    H_node unused;
    unused.conv_terminal();
    unused.next_leaf = this->leafs;
    this->leafs = &unused;

    // Init datas
    H_node* lists = NULL;
    for (H_node* node_now = this->leafs; node_now; node_now = node_now->next_leaf)
        lists = lists? lists->insert(node_now): node_now;

    // Compute length limit huffman tree
    for (u8 round = 0; round < 15; ++round) {
        H_node* lists2 = NULL;
        for (u8 i = 0; lists && lists->next && i < this->len; ++i) {
            H_node* merge  = new H_node();
            merge->pos     = lists;
            merge->neg     = lists->next;
            merge->counter = merge->pos->counter + merge->neg->counter;
            lists          = lists->next->next;
            lists2         = lists2? lists2->insert(merge): merge;
        }
        for (H_node* node_now = this->leafs; node_now; node_now = node_now->next_leaf)
            lists2 = lists2->insert(node_now);
        lists = lists2;
    }
    for (u8 i = 0; i < (this->len << 1); ++i) {
        lists->comp_depth();
        lists = lists->next;
    }

    // Remove unused
    this->leafs = this->leafs->next_leaf;
    // Sort with depth
    this->leafs = this->leafs->sort_by_depth(this->len);

    //printf("=================\n");
    u16 code = 0;
    u8  len = 0;
    for (H_node* node_now = this->leafs; node_now; node_now = node_now->next_leaf) {
        if (len != node_now->depth) {
            code = code << (node_now->depth - len);
            len  = node_now->depth;
        }
        node_now->terminal_data()->opt_code = code;
        node_now->terminal_data()->opt_len  = len;
        ++code;
        //printf("%x\t%d\t%x\n", node_now->terminal_data()->symbol,  \
                               //node_now->terminal_data()->opt_len, \
                               //node_now->terminal_data()->opt_code);
    }
}

void Huffman::write_opt (Stream* os) {
    H_node* node_now = this->leafs;
    for (u8 depth = 1; depth <= 16; ++depth) {
        u8 count = 0;
        for (; node_now && node_now->depth == depth; node_now = node_now->next_leaf)
            ++count;
        os->write_u8(count);
    }
    for (node_now = this->leafs; node_now; node_now = node_now->next_leaf)
        os->write_u8(node_now->terminal_data()->symbol);
}
