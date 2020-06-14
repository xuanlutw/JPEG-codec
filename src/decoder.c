# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <math.h>
# include <unistd.h>

// Define some constant
# define M_HEAD 0xFF
# define M_SOI  0xD8
# define M_APP0 0xE0
# define M_APP1 0xE1
# define M_APP2 0xE2
# define M_APP3 0xE3
# define M_APP4 0xE4
# define M_APP5 0xE5
# define M_APP6 0xE6
# define M_APP7 0xE7
# define M_APP8 0xE8
# define M_APP9 0xE9
# define M_APPA 0xEA
# define M_APPB 0xEB
# define M_APPC 0xEC
# define M_APPD 0xED
# define M_APPE 0xEE
# define M_APPF 0xEF
# define M_DQT  0xDB
# define M_SOF0 0xC0
# define M_DHT  0xC4
# define M_DRI  0xDD
# define M_SOS  0xDA
# define M_COM  0xFE
# define M_EOI  0xD9

# define N_DQT   16
# define N_QUANT 64
# define N_CHAN  5
# define N_DHT   16
# define N_HDEP  16

# define N_BLOCK 10

# define M_COS 200

# define TYPE_DC 0
# define TYPE_AC 1

# define FL_SKIP 1
# define FL_NSKIP 0

# define idx(i, j, h) (((i) * (h)) + (j))

# define check(cond, msg)           \
    do {                            \
        if (cond) {              \
            printf("%s\n", msg);    \
            exit(-1);               \
        }                           \
    } while(0);


// Define data type
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

typedef struct {
    u32 buffer;
    u8 len;
    u8 skip_fl;
    u32 counter;
    FILE* fp;
} F_BUFFER;

typedef struct {
    u8 precise;
    u16 quantizer[N_QUANT];
} DQT;

typedef struct {
    u8 h_factor;
    u8 v_factor;
    u8 N_BLOCK_MCU;
    u8 DQT_ID;
    u8 DC_table;
    u8 AC_table;
} CHAN;

typedef struct H_NODE {
    struct H_NODE* l;
    struct H_NODE* r;
} H_NODE;

typedef struct {
    H_NODE* root;
    H_NODE* pt;
    u8 input;
    u8 len;
    u8 symbol;
    u8 status;
} DHT;

typedef struct JPEG_INFO_ {
    DQT* DQT[N_DQT];
    u8 precise;
    u16 height;
    u16 width;
    u8 n_color;
    u16 DRI;
    CHAN* CHAN[N_CHAN];
    DHT* DC_DHT[N_DHT];
    DHT* AC_DHT[N_DHT];
    F_BUFFER* fp;
    u8 MCU_v_factor;
    u8 MCU_h_factor;
    u8 MCU_NV;
    u8 MCU_NH;
    u16 N_MCU;
    u16 MCU_height;
    u16 MCU_width;
} JPEG_INFO;

typedef double BLOCK[N_QUANT];

typedef struct {
    BLOCK** BLOCK;   // N_CHAN * N_BLOCK
} MCU;

typedef struct {
    MCU** MCU;
} JPEG_DATA;

typedef struct {
    u8* R;
    u8* G;
    u8* B;
    u16 width;
    u16 height;
} RGB;

// RGB file
RGB* init_RGB (u16 width, u16 height) {
    // Init RGB 

    RGB* obj = malloc(sizeof(RGB));
    obj->width  = width;
    obj->height = height;
    obj->R      = malloc(sizeof(u8) * width * height);
    obj->G      = malloc(sizeof(u8) * width * height);
    obj->B      = malloc(sizeof(u8) * width * height);
    return obj;
}

// Write BMP file
void write_u32 (FILE* fp, u32 num) {
    // Write little-endian u32

    for (u8 i = 0; i < 4; ++i) {
        fprintf(fp, "%c", num & 0xFF);
        num >>= 8;
    }
    return;
}

void write_u16 (FILE* fp, u16 num) {
    // Write little-endin u16

    for (u8 i = 0; i < 2; ++i) {
        fprintf(fp, "%c", num & 0xFF);
        num >>= 8;
    }
    return;
}

void write_bmp (char* filename, RGB* img) {
    // Write BMP file

    // Open file
    FILE* fp = fopen(filename, "w");
    check(fp == NULL, "Open BMP file fail");

    // Image data size with padding
    u32 img_size = img->height * ((img->width * 3 + 3) / 4) * 4;
    u32 padding_size = ((img->width * 3 + 3) / 4) * 4 - img->width * 3;

    // File Header
    fprintf(fp, "BM");              // Magic number
    write_u32(fp, 54 + img_size);   // File size, Header size = 54
    write_u32(fp, 0);               // Riverse 1 & 2
    write_u32(fp, 54);              // Offset

    // Info header
    write_u32(fp, 40);              // Ifo header size
    write_u32(fp, img->width);      // Width
    write_u32(fp, img->height);     // Height
    write_u16(fp, 1);               // # planes
    write_u16(fp, 24);              // # bits per pixel
    write_u32(fp, 0);               // NO compression
    write_u32(fp, img_size);        // Image size
    write_u32(fp, 100);             // Horizontal resolution
    write_u32(fp, 100);             // Vertical resolution
    write_u32(fp, 0);               // Number of color palette
    write_u32(fp, 0);               // Nnumber of important colors

    // Write pixel data
    for (i32 i = img->height - 1; i >= 0; --i) {
        for (u32 j = 0; j < img->width; ++j) {
            u32 index = idx(i, j, img->width);
            fprintf(fp, "%c%c%c", img->B[index], img->G[index], img->R[index]);
        }
        for (u16 j = 0; j < padding_size; ++j)
            fprintf(fp, "%c", 0);
    }
    fflush(fp);
    fclose(fp);

    return;
}

void write_ppm (RGB* img) {
    // Write ppm file
    // For test

    FILE* fp = fopen("test.ppm", "w");
    fprintf(fp, "P3\n%d %d\n255\n", img->width, img->height);
    for (u16 i = 0; i < img->height; ++i) {
        for (u16 j = 0; j < img->width; ++j) {
            u32 index = idx(i, j, img->width);
            fprintf(fp, "%d %d %d ", img->R[index], img->G[index], img->B[index]);
        }
        fprintf(fp, "\n");
    }
}

// Read file
u8 read_BUFFER (u16* content, u8 len, F_BUFFER* buf) {
    // Read at most 16 bits from buffer

    // Check length
    check(len > 16, "Length more then 16!");

    if (buf->len >= len) {
        // Read from biffer
        *content = (u16)((buf->buffer >> (buf->len - len)) & ((1 << len) - 1));
        buf->buffer  &= ((1 << (buf->len - len)) - 1);
        buf->len     -= len;
        buf->counter += len;
        return 1;
    }
    else {
        // Read from file
        u8 c;
        if (!fread(&c, sizeof(u8), 1, buf->fp))
            return 0;
        buf->buffer = (buf->buffer << 8) + c;
        buf->len += 8;
        if (buf->skip_fl == FL_SKIP && c == 0xFF) {
            fread(&c, sizeof(u8), 1, buf->fp);
            check(c != 0x00, "0xFF coding error\n");
        }
        return read_BUFFER(content, len, buf);
    }
}

void put_BUFFER (u8 content, u8 len, F_BUFFER* buf) {
    // Put bit to the buffer

    buf->buffer  += content << buf->len;
    buf->len     += len;
    buf->counter -= len;
}

u8 read_u8 (u8* content, F_BUFFER* buf) {
    // Read u8 from buffer

    u16 content16;
    u8 ret = read_BUFFER(&content16, 8, buf);
    *content = (u8)(content16);
    return ret;
}

u8 read_u16 (u16* content, F_BUFFER* buf) {
    // Read u16 from buffer

    return read_BUFFER(content, 16, buf);
}

u16 read_len (F_BUFFER* buf) {
    // Read u16

    u16 len;
    check(!read_u16(&len, buf), "Wrong length!");
    return len;
}

u32 get_counter (F_BUFFER* buf) {
    // Return number of bytes been read

    return (buf->counter) / 8;
}

void set_skip_fl (F_BUFFER* buf, u8 fl) {
    // Set the flag of skip 0xFFFF

    buf->skip_fl =fl;
    return;
}

F_BUFFER* init_F_BUFFER (char* file_path) {
    // Init the file buffer

    F_BUFFER* obj = malloc(sizeof(F_BUFFER));
    obj->fp       = fopen(file_path, "rb");
    obj->skip_fl  = FL_NSKIP;
    obj->len      = 0;
    obj->buffer   = 0;
    obj->counter  = 0;
    return obj;
}

// Decode Huffman code
u8 H_NODE_is_leaf (H_NODE* root) {
    // Check wheather the node is leaf

    return root->l == root;
}

u8 H_NODE_symbol (H_NODE* root) {
    // Get the symbol from the leaf node

    return (u8)(u64)root->r;
}

H_NODE* H_NODE_init () {
    // Init new node

    H_NODE* obj = malloc(sizeof(H_NODE));
    obj->l = NULL;
    obj->r = NULL;
    return obj;
}

void H_NODE_add_symbol (H_NODE* root, u16 content, u8 len, u8 symbol) {
    // Add new symbol into the tree

    // Check the root is not the leaf
    check(H_NODE_is_leaf(root), "Wrong tree construction!");

    // Add symbol
    if (len == 0) {
        // Directly
        root->l = root;
        root->r = (H_NODE*)(u64)symbol;
    }
    else {
        // Recursive
        if ((content >> (len - 1)) & 1) {
            // Add in right
            if (root->r == NULL)
                root->r = H_NODE_init();
            H_NODE_add_symbol(root->r, content, len - 1, symbol);
        }
        else {
            // Add in left
            if (root->l == NULL)
                root->l = H_NODE_init();
            H_NODE_add_symbol(root->l, content, len - 1, symbol);
        }
    }
    return;
}

void H_NODE_triverse (H_NODE* root, u16 content) {
    // Triverse the tree
    // For rest

    if (root->l == root) {
        // Is leaf
        printf("%x %d\n", content, (u8)(u64)root->r);
    }
    else {
        // Triverse
        if (root->l)
            // left
            H_NODE_triverse(root->l, content << 1);
        else
            printf("LEFT-NULLLLLLLLLLLLLLLL\n");
        if (root->r)
            // right
            H_NODE_triverse(root->r, (content << 1) + 1);
        else
            printf("RIGHT-NULLLLLLLLLLLLLLLL\n");
    }
}

// DHT
DHT* init_DHT () {
    // Init DHT

    DHT* obj = malloc(sizeof(DHT));
    obj->root = H_NODE_init();
    return obj;
}

void DHT_reset(DHT* table) {
    // Reset the status of the DHT

    table->len    = 0;
    table->pt     = table->root;
    table->status = 0;
}

u8 DHT_run(DHT* table) {
    // Run the DHT

    if (H_NODE_is_leaf(table->pt)) {
        // Touch the leaf
        table->symbol = H_NODE_symbol(table->pt);
        table->pt     = table->root;
        table->status = 1;
        return 1;
    }
    else if (table->len == 0) {
        // Out of the input
        table->status = 0;
        return 0;
    }
    else if (((table->input) >> (table->len - 1)) & 1)
        // Triverse right
        table->pt = table->pt->r;
    else
        // Triverse left
        table->pt = table->pt->l;
    table->len--;
    return DHT_run(table);
}

u8 DHT_put(DHT* table, u8 input, u8 len) {
    // Put input in the DHT

    table->input = input;
    table->len     = len;
    return DHT_run(table);
}

u8 DHT_get_symbol (DHT* table) {
    // Get the symbol from DHT

    return table->symbol;
}

u8 DHT_get_input (DHT* table) {
    // Get the input from DHT

    return table->input;
}

u8 DHT_get_len (DHT* table) {
    // Get the input length from DHT

    return table->len;
}

u8 DHT_get_status (DHT* table) {
    // Get the status of DHT

    return table->status;
}

// Run length code
u8 get_symbol_len (DHT* DHT_table, F_BUFFER* buf) {
    // Get the length of symbol from DHT

    DHT_reset(DHT_table);
    while (!DHT_get_status(DHT_table)) {
        // Recursive input until get symbol
        u8 c;
        read_u8(&c, buf);
        DHT_put(DHT_table, c, 8);
    }
    // Put the remain of the input back
    u8 remain_content = DHT_get_input(DHT_table);
    u8 remain_len = DHT_get_len(DHT_table);
    put_BUFFER(remain_content, remain_len, buf);
    return DHT_get_symbol(DHT_table);
}

i16 get_symbol (u8 len, F_BUFFER* buf) {
    // Get the symbol

    i16 c;
    read_BUFFER((u16*)&c, len, buf);
    // Decode
    if (c >> (len - 1))
        return c;
    else
        return -(c ^ ((1 << len) - 1));
}

// Zig-zag
void anti_zz (BLOCK obj) {
    // Inverse zig-zag code

    const u8 zz_map[N_QUANT] = {        \
         0,  1,  5,  6, 14, 15, 27, 28, \
         2,  4,  7, 13, 16, 26, 29, 42, \
         3,  8, 12, 17, 25, 30, 41, 43, \
         9, 11, 18, 24, 31, 40, 44, 53, \
        10, 19, 23, 32, 39, 45, 52, 54, \
        20, 22, 33, 38, 46, 51, 55, 60, \
        21, 34, 37, 47, 50, 56, 59, 61, \
        35, 36, 48, 49, 57, 58, 62, 63 };
    BLOCK tmp;
    for (u8 i = 0; i < N_QUANT; ++i)
        tmp[i] = obj[zz_map[i]];
    memcpy(obj, tmp, sizeof(BLOCK));
}

// Quantizer
void anti_q (BLOCK obj, DQT* quant) {
    // Inverse quantize

    for (u8 i = 0; i < N_QUANT; ++i)
        obj[i] *= quant->quantizer[i];
}

// IDCT
void fst_IDCT (BLOCK des, BLOCK src) {

    static double* cos_table = NULL;
    static double* coeff     = NULL;

    // Init cos table and only once
    if (!cos_table) {
        cos_table = malloc(sizeof(double) * M_COS);
        for (u8 i = 0; i < M_COS; i++)
            cos_table[i] = cos(i * M_PI / 16.0);
    }

    // Init coefficient and only once
    if (!coeff) {
        coeff = malloc(sizeof(double) * 8);
        coeff[0] = 1. / sqrt(2.);
        for (u8 i = 1; i < 8; i++)
            coeff[i] = 1.;
    }

    // Transform
    memset(des, 0, sizeof(double) * N_QUANT);
    for (int j = 0; j < 8; j++)
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++)
                des[idx(j, x, 8)] += coeff[y] * src[idx(x, y, 8)] * cos_table[((j << 1) + 1) * y];
            des[idx(j, x, 8)] /= 2.;
        }
    return;
}

void IDCT (BLOCK obj) {
    // Inverse descrete cosine transform

    BLOCK tmp;
    fst_IDCT(tmp, obj);
    fst_IDCT(obj, tmp);
}

// Sampling
void upsampling(MCU* obj, JPEG_INFO* info) {
    // Reconstruct full data block

    for (u8 chan = 1; chan <= 3; ++chan) {
        if ((info->CHAN[chan]->v_factor == info->MCU_v_factor) && \
            (info->CHAN[chan]->h_factor == info->MCU_h_factor))
            // Same sample rate, skip
            continue;
        u8 MCU_N_BLOCK = info->MCU_v_factor * info->MCU_h_factor;
        BLOCK* up[MCU_N_BLOCK];
        for (u8 i = 0; i < MCU_N_BLOCK; ++i)
            up[i] = malloc(sizeof(BLOCK));

        for (u8 i = 0; i < info->MCU_v_factor; ++i) {
            for (u8 j = 0; j < info->MCU_h_factor; ++j) {
                for (u8 k = 0; k < 8; ++k) {
                    for (u8 l = 0; l < 8; ++l) {
                        u8 x = (8 * i + k) * info->CHAN[chan]->v_factor / info->MCU_v_factor;
                        u8 y = (8 * j + l) * info->CHAN[chan]->h_factor / info->MCU_h_factor;
                        (*up[info->MCU_h_factor * i + j])[8 * k + l] = \
                            (*obj->BLOCK[idx(chan, (x / 8) * info->CHAN[chan]->h_factor + (y / 8), N_CHAN)])\
                            [8 * (x % 8) + (y % 8)];
                    }
                }
            }
        }
        for (u8 i = 0; i < MCU_N_BLOCK; ++i)
            obj->BLOCK[idx(chan, i, N_CHAN)] = up[i];
    }
}

// Color
u8 bandpass (double val) {
    // Bandpass filter

    if (val >= 255.0)
        return 255;
    else if (val <= 0.0)
        return 0;
    else
        return (u8) round(val);
}

RGB* anti_trans_color (JPEG_INFO* info, JPEG_DATA* data) {
    // Anti color transform

    RGB* img = init_RGB(info->width, info->height);
    for (u8 i = 0; i < info->MCU_NV; ++i)
        for (u8 j = 0; j < info->MCU_NH; ++j)
            for (u8 k = 0; k < info->MCU_v_factor; ++k)
                for (u8 l = 0; l < info->MCU_h_factor; ++l)
                    for (u8 m = 0; m < 8; ++m)
                        for (u8 n = 0; n < 8; ++n) {
                            u16 x = 8 * info->MCU_v_factor * i + 8 * k + m;
                            u16 y = 8 * info->MCU_h_factor * j + 8 * l + n;
                            if (x >= info->height || y >= info->width)
                                continue;
                            u16 idx_MCU   = info->MCU_NH * i + j;
                            u16 idx_BLOCK = info->MCU_h_factor * k + l;
                            u16 idx_px    = 8 * m + n;
                            u32 index     = idx(x, y, info->width);
                            double Y  = (*data->MCU[idx_MCU]->BLOCK[idx(1, idx_BLOCK, N_CHAN)])[idx_px];
                            double C1 = (*data->MCU[idx_MCU]->BLOCK[idx(2, idx_BLOCK, N_CHAN)])[idx_px];
                            double C2 = (*data->MCU[idx_MCU]->BLOCK[idx(3, idx_BLOCK, N_CHAN)])[idx_px];
                            img->R[index] = bandpass(1.0 * Y + 1.402 * C2 + 128.0);
                            img->G[index] = bandpass(1.0 * Y - 0.34414 * C1 - 0.71414 * C2 + 128.0);
                            img->B[index] = bandpass(1.0 * Y + 1.772 * C1 + 128.0);
                        }
    return img;
}

// Read raw data
BLOCK* read_BLOCK (JPEG_INFO* info, u8 chan_id) {
    // Read one block from the info

    DHT* DC_table = info->DC_DHT[info->CHAN[chan_id]->DC_table];
    DHT* AC_table = info->AC_DHT[info->CHAN[chan_id]->AC_table];
    BLOCK* obj = malloc(sizeof(BLOCK));
    memset(obj, 0, sizeof(BLOCK));
    u8 symbol_len;
    static i16 DC_bias[N_CHAN] = {0};   // For DC bias

    // Read DC
    symbol_len = get_symbol_len(DC_table, info->fp);
    (*obj)[0] = get_symbol(symbol_len, info->fp) + DC_bias[chan_id];
    DC_bias[chan_id] = (*obj)[0];

    // Read AC
    for (u8 counter = 1; counter < N_QUANT; ++counter) {
        symbol_len = get_symbol_len(AC_table, info->fp);
        if (symbol_len == 0x00)
            break;
        else if (symbol_len == 0xF0)
            counter += 15;
        else {
            counter += symbol_len >> 4;
            (*obj)[counter] = get_symbol(symbol_len & 0x0F, info->fp);
        }
    }
    return obj;
}


MCU* read_MCU (JPEG_INFO* info) {
    // Read one MCU and do some processing

    MCU* obj = malloc(sizeof(MCU));
    obj->BLOCK = malloc(sizeof(BLOCK*) * N_CHAN * N_BLOCK);
    for (u8 chan = 1; chan <= 3; ++chan) {
        for (u8 i = 0; i < info->CHAN[chan]->N_BLOCK_MCU; ++i) {
            u16 index = idx(chan, i, N_CHAN);
            obj->BLOCK[index] = read_BLOCK(info, chan);
            anti_q((double*)obj->BLOCK[index], info->DQT[info->CHAN[chan]->DQT_ID]);
            anti_zz((double*)obj->BLOCK[index]);
            IDCT((double*)obj->BLOCK[index]);
        }
    }
    upsampling(obj, info);
    return obj;
}

RGB* read_JPEG_img (JPEG_INFO* info) {
    // Read whole image

    JPEG_DATA obj;
    obj.MCU = malloc(sizeof(MCU*) * info->N_MCU);
    set_skip_fl(info->fp, FL_SKIP);
    for (u16 i = 0; i < info->N_MCU; ++i) {
        obj.MCU[i] = read_MCU(info);
    }
    set_skip_fl (info->fp, FL_NSKIP);
    info->fp->len = (info->fp->len / 8) * 8; //ALIGN
    RGB* img = anti_trans_color(info, &obj);
    return img;
}

// JPEG Info
JPEG_INFO* init_JPEG_INFO (char* jpeg_path) {
    // init JPEG_INFO

    JPEG_INFO* obj = malloc(sizeof(JPEG_INFO));
    obj->fp           = init_F_BUFFER(jpeg_path);
    obj->DRI          = 0;
    obj->MCU_v_factor = 0;
    obj->MCU_h_factor = 0;
    for (int i = 0;i < N_DQT;++i)
        obj->DQT[i] = NULL;
    for (int i = 0;i < N_CHAN;++i)
        obj->CHAN[i] = NULL;
    return obj;
}

void read_APP0 (JPEG_INFO* info) {
    // Read data from APP0

    u16 len = read_len(info->fp);
    u8 c;

    // Skip all data
    for (int i = 0; i < len - 2; ++i)
        read_u8(&c, info->fp);
    return;
}

void read_APPn (JPEG_INFO* info) {
    // Read data from APPn

    u16 len = read_len(info->fp);
    u8 c;

    // Skip all data
    for (int i = 0; i < len - 2; ++i)
        read_u8(&c, info->fp);
    return;
}

void read_DQT (JPEG_INFO* info) {
    // Read quantizer

    u8 val;
    u16 len, repeat;
    // Read the length and check size
    read_u16(&len, info->fp);
    repeat = (len - 2) / (N_QUANT + 1);
    check(repeat * (N_QUANT + 1) + 2 != len, "Wrong DQT size");

    // Read quantizer
    for (u16 i = 0; i < repeat; ++i) {
        read_u8(&val, info->fp);
        DQT* obj = malloc(sizeof(DQT));
        obj->precise = val >> 4;
        for (int j = 0; j < N_QUANT; ++j)
            if (obj->precise == 0)
                read_u8((u8*)(obj->quantizer + j), info->fp);
            else
                read_u16(obj->quantizer + j, info->fp);
        info->DQT[val & 0x0F] = obj;
    }
}

void read_SOF0 (JPEG_INFO* info) {
    // Read SOF0

    u8 id;
    u16 len = read_len(info->fp);
    read_u8(&(info->precise), info->fp);
    read_u16(&(info->height), info->fp);
    read_u16(&(info->width), info->fp);
    read_u8(&(info->n_color), info->fp);

    // Check size
    check(8 + 3 * info->n_color != len, "Wrong SOF0 size");

    // Readd color table
    for (int i = 0; i < info->n_color; ++i) {
        CHAN* obj = malloc(sizeof(CHAN));
        read_u8(&id, info->fp);
        info->CHAN[id] = obj;
        read_u8(&(obj->h_factor), info->fp);
        read_u8(&(obj->DQT_ID), info->fp);
        obj->v_factor = obj->h_factor & 0x0F;
        obj->h_factor = obj->h_factor >> 4;
        obj->N_BLOCK_MCU = (obj->v_factor) * (obj->h_factor);
        if (obj->v_factor > info->MCU_v_factor)
            info->MCU_v_factor = obj->v_factor;
        if (obj->h_factor > info->MCU_h_factor)
            info->MCU_h_factor = obj->h_factor;
    }
    info->MCU_NV = (info->height + info->MCU_v_factor * 8 - 1) / (info->MCU_v_factor * 8);
    info->MCU_NH = (info->width + info->MCU_h_factor * 8 - 1) / (info->MCU_h_factor * 8);
    info->N_MCU  = info->MCU_NV * info->MCU_NH;
    info->MCU_height = (info->MCU_v_factor) * (info->MCU_NV) * 8;
    info->MCU_width  = (info->MCU_h_factor) * (info->MCU_NH) * 8;
}

void read_DHT (JPEG_INFO* info) {
    // Read DHT

    u16 len = read_len(info->fp);
    len -= 2;

    // Read ALL DHT
    while (1) {
        if (len == 0)
            break;
        check(len & 0x8000, "DHT len error!\n");
        DHT* obj = init_DHT();
        u16 counter = 0;
        u8 c_len = 0xFF;
        u16 content;
        u8 symbol;
        u8 num[N_HDEP];
        u8 c;

        // Read DHT type
        read_u8(&c, info->fp);
        if (c >> 4 == TYPE_DC)
            info->DC_DHT[c & 0x0F] = obj;
        else if (c >> 4 == TYPE_AC)
            info->AC_DHT[c & 0x0F] = obj;

        // Read info
        for (int i = 0; i < N_HDEP; ++i) {
            read_u8(num + i, info->fp);
            counter += num[i];
            if (num[i] != 0 && c_len == 0xFF)
                c_len = i;
        }

        // Construct tree
        content = 0;
        for (int i = 0; i < counter; ++i) {
            read_u8(&symbol, info->fp);
            H_NODE_add_symbol(obj->root, content, c_len + 1, symbol);
            num[c_len]--;
            content++;
            while (!num[c_len] && i != counter - 1) {
                c_len++;
                content = content << 1;
            }
        }
        len = len - 17 - counter;

        // TEST
        /*printf("TYPE: %d %d\n", c >> 4, c & 0x0F);*/
        /*H_NODE_triverse(obj->root, 0);*/
        /*DHT_reset(obj);*/
        /*printf("%d ", DHT_put(obj, 0, 8));*/
        /*printf("%d\n", DHT_get_symbol(obj));*/
    }
}

void read_DRI (JPEG_INFO* info) {
    // Read DRI

    u16 len = read_len(info->fp);
    check(len != 4, "DRI len error!\n");
    read_u16(&(info->DRI), info->fp);
}

void read_COM (JPEG_INFO* info) {
    // Read comment

    u16 len = read_len(info->fp);
    u8 c;
    printf("===== Comment =====\n");
    for (u16 i = 0; i < len - 2; ++i) {
        read_u8(&c, info->fp);
        printf("%c", c);
    }
    printf("===================\n");
}

void read_SOS (JPEG_INFO* info) {
    // Read SOS

    u8 tmp;
    u16 len = read_len(info->fp);
    len -= 2;

    // Read channel info
    read_u8(&(info->n_color), info->fp);
    for (int i = 0; i < (len - 4) / 2; ++i) {
        u8 ID;
        u8 table;
        read_u8(&ID, info->fp);
        read_u8(&table, info->fp);
        info->CHAN[ID]->DC_table = table >> 4;
        info->CHAN[ID]->AC_table = table & 0x0F;
        /*printf("%d %d\n", ID, table >> 4);*/
    }

    // Check magic number
    read_u8(&tmp, info->fp);
    check(tmp != 0x00, "Wrong SOS-1\n");
    read_u8(&tmp, info->fp);
    check(tmp != 0x3F, "Wrong SOS-2\n");
    read_u8(&tmp, info->fp);
    check(tmp != 0x00, "Wrong SOS-3\n");
}

JPEG_INFO* read_JPEG_INFO (char* jpeg_path) {
    // Read ALL header

    JPEG_INFO* info = init_JPEG_INFO(jpeg_path);
    u8 val_now;

    // Parse header
    while (read_u8(&val_now, info->fp)) {
        if (val_now == M_HEAD) {
            read_u8(&val_now, info->fp);
            switch (val_now) {
                case M_SOI:
                    printf("SOI at %d\n", get_counter(info->fp));
                    break;
                case M_EOI:
                    printf("EOI at %d\n", get_counter(info->fp));
                    break;
                case M_APP0:
                    read_APP0(info);
                    printf("APP0 at %d\n", get_counter(info->fp));
                    break;
                case M_APP1:
                case M_APP2:
                case M_APP3:
                case M_APP4:
                case M_APP5:
                case M_APP6:
                case M_APP7:
                case M_APP8:
                case M_APP9:
                case M_APPA:
                case M_APPB:
                case M_APPC:
                case M_APPD:
                case M_APPE:
                case M_APPF:
                    read_APPn(info);
                    printf("APPn at %d\n", get_counter(info->fp));
                    break;
                case M_DQT:
                    read_DQT(info);
                    printf("DQT at %d\n", get_counter(info->fp));
                    break;
                case M_SOF0:
                    read_SOF0(info);
                    printf("SOF0 at %d\n", get_counter(info->fp));
                    break;
                case M_DHT:
                    read_DHT(info);
                    printf("DHT at %d\n", get_counter(info->fp));
                    break;
                case M_DRI:
                    read_DRI(info);
                    printf("DRI at %d\n", get_counter(info->fp));
                    break;
                case M_COM:
                    read_COM(info);
                    printf("COM at %d\n", get_counter(info->fp));
                    break;
                case M_SOS:
                    read_SOS(info);
                    printf("SOS at %d\n", get_counter(info->fp));
                    return info;
                case 0:
                default:
                    printf("Unknow marker at %d, %x\n", get_counter(info->fp), val_now);
            }
        }
        else {
            printf("Unknow byte at %d, val = %x\n", get_counter(info->fp), val_now);
        }
    }

    // DEBUG
    /*for (int i = 0; i < N_DQT; ++i) {*/
        /*if (!info->DQT[i])*/
            /*break;*/
        /*printf("%d\n", i);*/
        /*for (int j = 0; j < N_QUANT; ++j) */
            /*printf("%d ", info->DQT[i]->quantizer[j]);*/
        /*printf("\n");*/
    /*}*/
    return info;
}

int main (int argc, char* argv[]) {

    // Check command line parameter
    if (argc !=3) {
        printf("Usage\n\t %s jpeg_path bmp_path\n", argv[0]);
        exit(1);
    }

    // Read File
    JPEG_INFO* info = read_JPEG_INFO(argv[1]);
    RGB* img = read_JPEG_img(info);
    write_bmp(argv[2], img);
    /*write_ppm(img);*/
}
