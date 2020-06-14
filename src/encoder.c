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
# define M_DQT  0xDB
# define M_SOF0 0xC0
# define M_DHT  0xC4
# define M_DRI  0xDD
# define M_SOS  0xDA
# define M_EOI  0xD9

# define N_QUANT 64
# define N_BLOCK 10
# define M_COS 200

# define TYPE_DC 0
# define TYPE_AC 1

# define FL_SKIP 1
# define FL_NSKIP 0

# define DQT_Y_ID 0
# define DQT_C_ID 1

# define DHT_Y_ID 0
# define DHT_C_ID 1

# define idx(i, j, h) (((i) * (h)) + (j))

# define check(cond, msg)           \
    do {                            \
        if (cond) {                 \
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
    u64 counter;
    FILE* fp;
} F_BUFFER;

typedef struct {
    u8 precise;
    u16 quantizer[N_QUANT];
} DQT;

typedef struct {
    u16 num_symbol;
    u8 num[16];
    u8 symbol[];
} pre_DHT;

typedef struct {
    u16 content[256];
    u8 len[256];
} DHT_r;

typedef struct {
    u8* R;
    u8* G;
    u8* B;
    u16 width;
    u16 height;
} RGB;

// Define compress parameter
DQT DQT_Y = {
    .precise = 0,
    .quantizer = {
         3,  2,  2,  2,  2,  2,  3,  2,
         2,  2,  3,  3,  3,  3,  4,  6,
         4,  4,  4,  4,  4,  8,  6,  6,
         5,  6,  9,  8, 10, 10,  9,  8,
         9,  9, 10, 12, 15, 12, 10, 11,
        14, 11,  9,  9, 13, 17, 13, 14,
        15, 16, 16, 17, 16, 10, 12, 18,
        19, 18, 16, 19, 15, 16, 16, 16
    }
};

DQT DQT_C = {
    .precise = 0,
    .quantizer = {
         3,  3,  3,  4,  3,  4,  8,  4,
         4,  8, 16, 11,  9, 11, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16
    }
};

pre_DHT pre_DHT_DC_Y = {
    .num_symbol = 12,
    .num = {0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    .symbol = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b}
};

pre_DHT pre_DHT_DC_C = {
    .num_symbol = 12,
    .num = {0x00,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00},
    .symbol = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b}
};

pre_DHT pre_DHT_AC_Y = {
    .num_symbol = 162,
    .num = {0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,0x02,0x77},
    .symbol = {0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,
               0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,
               0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,
               0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
               0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
               0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
               0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
               0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,
               0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
               0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
               0xF9,0xFA}
};

pre_DHT pre_DHT_AC_C= {
    .num_symbol = 162,
    .num = {0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,0x00,0x01,0x7d},
    .symbol = {0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
               0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,
               0x15,0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,
               0x27,0x28,0x29,0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,
               0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,
               0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,
               0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,
               0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,
               0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,
               0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
               0xF9,0xFA}
};

// Raw data
RGB* init_RGB (u16 width, u16 height) {
    // Init RGB data

    RGB* obj = malloc(sizeof(RGB));
    obj->width  = width;
    obj->height = height;
    obj->R      = malloc(sizeof(u8) * width * height);
    obj->G      = malloc(sizeof(u8) * width * height);
    obj->B      = malloc(sizeof(u8) * width * height);
    return obj;
}

// Process bmp file
u32 read_u32 (FILE* fp) {
    // Read little-endian u32

    u8 c;
    u32 num = 0;

    for (u8 i = 0; i < 4; ++i) {
        fscanf(fp, "%c", &c);
        num = num + ((u32)c << (8 * i));
    }
    return num;
}

u16 read_u16 (FILE* fp) {
    // Read little-endin u16

    u8 c;
    u16 num = 0;

    for (u8 i = 0; i < 2; ++i) {
        fscanf(fp, "%c", &c);
        num = num + ((u16)c << (8 * i));
    }
    return num;
}

u8 read_u8 (FILE* fp) {
    // Read little-endin u16

    u8 num;

    fscanf(fp, "%c", &num);
    return num;
}

RGB* read_bmp (char* filename) {
    // Write BMP file

    // Open file
    FILE* fp = fopen(filename, "r");
    check(fp == NULL, "Open BMP file fail");

    u8 c1;
    u8 c2;
    u16 bits;
    u32 tmp;
    u32 offset;
    u32 width;
    u32 height;
    u32 padding_size;

    // File Header
    c1 = read_u8(fp);       // Magic Number
    c2 = read_u8(fp);
    read_u32(fp);           // File size, Header size = 54
    read_u32(fp);           // Riverse 1 & 2
    offset = read_u32(fp);  // Offset
    check(c1 != 'B', "Wrong BMP file format");
    check(c2 != 'M', "Wrong BMP file format");

    // Info header
    read_u32(fp);               // Ifo header size
    width  = read_u32(fp);      // Width
    height = read_u32(fp);      // Height
    read_u16(fp);               // # planes
    bits = read_u16(fp);        // # bits per pixel

    // Check info
    check(bits != 24 && bits != 32, "BMP bits per pixel should be 24 or 32!");
    if (bits == 24)
        padding_size = ((width * 3 + 3) / 4) * 4 - width * 3;
    else if (bits == 32)
        padding_size = 0;
    RGB* img = init_RGB(width, height);

    // Read pixel data
    fseek(fp, offset, SEEK_SET);
    for (i32 i = img->height - 1; i >= 0; --i) {
        for (u32 j = 0; j < img->width; ++j) {
            u32 index = idx(i, j, img->width);
            if (bits == 24)
                fscanf(fp, "%c%c%c", img->B + index, img->G + index, img->R + index);
            else
                fscanf(fp, "%c%c%c%*c", img->B + index, img->G + index, img->R + index);
        }
        for (u16 j = 0; j < padding_size; ++j)
            fscanf(fp, "%*c");
    }
    fclose(fp);

    return img;
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

// Write file
void run_BUFFER (F_BUFFER* buf) {
    // Write down buffers

    if (buf->len >= 8) {
        // Check size
        u8 to_write = (buf->buffer >> (buf->len - 8)) & 0xFF;
        buf->len -= 8;
        fwrite(&to_write, sizeof(u8), 1, buf->fp);
        if (buf->skip_fl == FL_SKIP && to_write == 0xFF) {
            u8 c = 0;
            fwrite(&c, sizeof(u8), 1, buf->fp);
        }
        run_BUFFER(buf);
    }
}
void write_BUFFER (u16 content, u8 len, F_BUFFER* buf) {
    // Write at most 32 bits to buffer

    // Check length
    check(len > 32, "Length more then 16!");

    // Add new data
    buf->buffer = (buf->buffer << len) + (content & ((1 << len) - 1));
    buf->len += len;
    run_BUFFER(buf);
}

void flush_BUFFER (F_BUFFER* buf) {
    // Write all remain data

    run_BUFFER(buf);
    if (buf->len != 0)
        write_BUFFER(0, 8 - (buf->len), buf);
    run_BUFFER(buf);
}

void write_u8 (u8 content, F_BUFFER* buf) {
    // Write u8 to buffer

    write_BUFFER(content, 8, buf);
}

void write_u16 (u16 content, F_BUFFER* buf) {
    // Write u16 from buffer

    write_BUFFER(content, 16, buf);
}

void write_header (u8 header, F_BUFFER* buf) {
    // Write JPEG header

    write_u8(M_HEAD, buf);
    write_u8(header, buf);
}

void set_skip_fl (F_BUFFER* buf, u8 fl) {
    // Set the flag of skip 0xFFFF

    buf->skip_fl =fl;
    return;
}

F_BUFFER* init_F_BUFFER (char* file_path) {
    // Init the file buffer

    F_BUFFER* obj = malloc(sizeof(F_BUFFER));
    obj->fp       = fopen(file_path, "wb");
    obj->skip_fl  = FL_NSKIP;
    obj->len      = 0;
    obj->buffer   = 0;
    obj->counter  = 0;
    return obj;
}

// Process DHT data
DHT_r* init_DHT_r(pre_DHT* pre) {
    // Init DHT reverse data struct

    DHT_r* obj = malloc(sizeof(DHT_r));
    memset(obj->len, 0, sizeof(u8) * 256);

    // Get the first non empty entry
    u16 content = 0;
    u8 len = 0;
    for (u8 i = 0; i < 16; ++i)
        if (pre->num[i] != 0) {
            len = i;
            break;
        }

    // Get the content of all
    for (u16 i = 0; i < pre->num_symbol; ++i) {
        u8 symbol = pre->symbol[i];
        obj->len[symbol] = len + 1;
        obj->content[symbol] = content;
        pre->num[len]--;
        content++;
        while (!pre->num[len] && i != pre->num_symbol - 1) {
            len++;
            content = content << 1;
        }
    }
    return obj;
}

// Processing Data
void subsampling (double* img, u16 width, u16 height, u8 color_v_fact, u8 color_h_fact) {
    // Do subsampling

    // Width
    for (u16 i = 0; i < width / color_h_fact; ++i)
        for (u16 j = 0; j < height; ++j) {
            double tmp = 0;
            for (u8 k = 0; k < color_h_fact; ++k) {
                u32 index = idx(j, color_h_fact * i + k, width);
                tmp += img[index];
            }
            u32 index = idx(j, i, width);
            img[index] = tmp / color_h_fact;
        }

    // Height
    for (u16 i = 0; i < height / color_v_fact; ++i)
        for (u16 j = 0; j < width; ++j) {
            double tmp = 0;
            for (u8 k = 0; k < color_v_fact; ++k) {
                u32 index = idx(color_v_fact * i + k, j, width);
                tmp += img[index];
            }
            u32 index = idx(i, j, width);
            img[index] = tmp / color_v_fact;
        }
}

void DCT (double* img, u16 width, u16 base_x, u16 base_y) {
    // Descrete cosine transform

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
    double tmp[N_QUANT];
    memset(tmp, 0, sizeof(double) * N_QUANT);
    for (int j = 0; j < 8; j++)
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++)
                tmp[idx(j, x, 8)] += img[idx(j + base_x, y + base_y, width)] * cos_table[((y << 1) + 1) * x];
            tmp[idx(j, x, 8)] *= coeff[x] / 2.;
        }
    for (int j = 0; j < 8; j++)
        for (int x = 0; x < 8; x++) {
            img[idx(j + base_x, x + base_y, width)] = 0;
            for (int y = 0; y < 8; y++)
                img[idx(j + base_x, x + base_y, width)] += tmp[idx(y, x, 8)] * cos_table[((y << 1) + 1) * j];
            img[idx(j + base_x, x + base_y, width)] *= coeff[j] / 2.;
        }
    return;
}

void quantize (double* src, i16* des, u16 width, u16 base_x, u16 base_y, DQT* q) {
    // Quantizer and zigzag

    const u8 zz_map[N_QUANT] = {        \
         0,  1,  5,  6, 14, 15, 27, 28, \
         2,  4,  7, 13, 16, 26, 29, 42, \
         3,  8, 12, 17, 25, 30, 41, 43, \
         9, 11, 18, 24, 31, 40, 44, 53, \
        10, 19, 23, 32, 39, 45, 52, 54, \
        20, 22, 33, 38, 46, 51, 55, 60, \
        21, 34, 37, 47, 50, 56, 59, 61, \
        35, 36, 48, 49, 57, 58, 62, 63 };
    i16 tmp[N_QUANT];
    for (u8 i = 0; i < 8; ++i)
        for (u8 j = 0; j < 8; ++j)
            tmp[zz_map[idx(i, j, 8)]] = src[idx(i + base_x, j + base_y, width)] / q->quantizer[zz_map[idx(i, j, 8)]];
    for (u8 i = 0; i < 8; ++i)
        for (u8 j = 0; j < 8; ++j)
            des[idx(i + base_x, j + base_y, width)] = round(tmp[idx(i, j, 8)]);
}

void DPCM (i16 symbol, u8* len, u16* content) {
    // Get DPCM

    u16 tmp;
    // Compute content
    if (symbol < 0) {
        tmp = -symbol;
        *content = ~tmp;
    }
    else {
        tmp = symbol;
        *content = tmp;
    }

    // Compute length
    *len = 0;
    for (u8 i = 0; i < 16;++i)
        if (!tmp) {
            *len = i;
            break;
        }
        else
            tmp >>= 1;
}

void write_block(i16* img, u8 num_chn, u16 base_x, u16 base_y, u16 width, F_BUFFER* fp,\
        DHT_r* DHT_r_DC, DHT_r*DHT_r_AC) {
    // Encode block and write

    // Extract data
    u8 len;
    u16 content;
    i16 tmp[N_QUANT];
    for (u8 i = 0; i < 8; ++i)
        for (u8 j = 0; j < 8; ++j)
            tmp[idx(i, j, 8)] = img[idx(base_x + i, base_y + j, width)];

    // Process DC
    static i16 pre[4] = {0};
    i16 diff = tmp[0] - pre[num_chn];
    pre[num_chn] = tmp[0];
    DPCM(diff, &len, &content);
    write_BUFFER(DHT_r_DC->content[len], DHT_r_DC->len[len], fp);
    write_BUFFER(content, len, fp);

    // Process AC
    // Find last non zero term
    u8 last_non_zero = 0;
    for (u8 i = N_QUANT - 1; i > 0; --i)
        if (tmp[i]) {
            last_non_zero = i;
            break;
        }
    // Run length encode
    u8 zero_count = 0;
    for (u8 counter = 1; counter <= last_non_zero; ++counter) {
        if (zero_count == 16) {
            write_BUFFER(DHT_r_AC->content[0xF0], DHT_r_AC->len[0xF0], fp);
            zero_count = 0;
        }
        if (tmp[counter] == 0) {
            zero_count++;
            continue;
        }
        DPCM(tmp[counter], &len, &content);
        u8 to_write = (zero_count << 4) + (len & 0x0F);
        write_BUFFER(DHT_r_AC->content[to_write], DHT_r_AC->len[to_write], fp);
        write_BUFFER(content, len, fp);
        zero_count = 0;
    }
    // Truncate zero
    if (last_non_zero != N_QUANT)
        write_BUFFER(DHT_r_AC->content[0x00], DHT_r_AC->len[0x00], fp);
}

void write_JPEG_data (RGB* img, u8 Y_v_rate, u8 Y_h_rate, u8 C_v_rate, u8 C_h_rate, F_BUFFER* fp) {
    // Write compress data

    // Compute some parameter
    u8 MCU_v_rate;
    u8 MCU_h_rate;

    if (Y_v_rate > C_v_rate)
        MCU_v_rate = Y_v_rate;
    else
        MCU_v_rate = C_v_rate;

    if (Y_h_rate > C_h_rate)
        MCU_h_rate = Y_h_rate;
    else
        MCU_h_rate = C_h_rate;

    u16 N_MCU_v = (img->height + 8 * MCU_v_rate - 1) / (8 * MCU_v_rate);
    u16 N_MCU_h = (img->width + 8 * MCU_h_rate - 1) / (8 * MCU_h_rate);

    u16 width_ext  = N_MCU_h * 8 * MCU_h_rate;
    u16 height_ext = N_MCU_v * 8 * MCU_v_rate;

    /*printf("%d %d %d %d\n", img->width, img->height, width_ext, height_ext);*/

    u16 N_BLOCK_v = height_ext / 8;
    u16 N_BLOCK_h = width_ext  / 8;

    u8 Y_v_fact = MCU_v_rate / Y_v_rate;
    u8 Y_h_fact = MCU_h_rate / Y_h_rate;
    u8 C_v_fact = MCU_v_rate / C_v_rate;
    u8 C_h_fact = MCU_h_rate / C_h_rate;

    // Some variable
    double Y[width_ext * height_ext];
    double Cb[width_ext * height_ext];
    double Cr[width_ext * height_ext];
    i16 Y8[width_ext * height_ext];
    i16 Cb8[width_ext * height_ext];
    i16 Cr8[width_ext * height_ext];
    DHT_r* DHT_r_DC_Y = init_DHT_r(&pre_DHT_DC_Y);
    DHT_r* DHT_r_AC_Y = init_DHT_r(&pre_DHT_AC_Y);
    DHT_r* DHT_r_DC_C = init_DHT_r(&pre_DHT_DC_C);
    DHT_r* DHT_r_AC_C = init_DHT_r(&pre_DHT_AC_C);

    // Color conversion
    for (u16 i = 0; i < height_ext; ++i)
        for (u16 j = 0; j < width_ext; ++j) {
            u32 index     = idx(i, j, img->width);
            u32 index_ext = idx(i, j, width_ext);
            if (i < img->height && j < img->width) {
                Y[index_ext]  = 0.299 * ((double)img->R[index] - 128.) + \
                                0.587 * ((double)img->G[index] - 128.) + \
                                0.114 * ((double)img->B[index] - 128.);
                Cb[index_ext] =-0.168 * ((double)img->R[index] - 128.) + \
                               -0.331 * ((double)img->G[index] - 128.) + \
                                0.500 * ((double)img->B[index] - 128.);
                Cr[index_ext] = 0.500 * ((double)img->R[index] - 128.) + \
                               -0.419 * ((double)img->G[index] - 128.) + \
                               -0.081 * ((double)img->B[index] - 128.);
            }
            else {
                Y[index_ext]  = 0;
                Cb[index_ext] = 0;
                Cr[index_ext] = 0;
            }
        }

    // Subsampling
    subsampling(Y,  width_ext, height_ext, Y_v_fact, Y_h_fact);
    subsampling(Cb, width_ext, height_ext, C_v_fact, C_h_fact);
    subsampling(Cr, width_ext, height_ext, C_v_fact, C_h_fact);

    // Encode and Write MCU
    set_skip_fl(fp, FL_SKIP);
    for (u16 i = 0; i < N_MCU_v; ++i)
        for (u16 j = 0; j < N_MCU_h; ++j) {
            // Write Y
            for (u16 k = 0; k < Y_v_rate; ++k)
                for (u16 l = 0; l < Y_h_rate; ++l) {
                    u16 base_x = (i * Y_v_rate + k) * 8;
                    u16 base_y = (j * Y_h_rate + l) * 8;
                    DCT(Y, width_ext, base_x, base_y);
                    quantize(Y, Y8, width_ext, base_x, base_y, &DQT_Y);
                    write_block(Y8, 1, base_x, base_y, width_ext, fp, DHT_r_DC_Y, DHT_r_AC_Y);
                }
            // Write Cb
            for (u16 k = 0; k < C_v_rate; ++k)
                for (u16 l = 0; l < C_h_rate; ++l) {
                    u16 base_x = (i * C_v_rate + k) * 8;
                    u16 base_y = (j * C_h_rate + l) * 8;
                    DCT(Cb, width_ext, base_x, base_y);
                    quantize(Cb, Cb8, width_ext, base_x, base_y, &DQT_C);
                    write_block(Cb8, 2, base_x, base_y, width_ext, fp, DHT_r_DC_C, DHT_r_AC_C);
                }
            // Write Cr
            for (u16 k = 0; k < C_v_rate; ++k)
                for (u16 l = 0; l < C_h_rate; ++l) {
                    u16 base_x = (i * C_v_rate + k) * 8;
                    u16 base_y = (j * C_h_rate + l) * 8;
                    DCT(Cr, width_ext, base_x, base_y);
                    quantize(Cr, Cr8, width_ext, base_x, base_y, &DQT_C);
                    write_block(Cr8, 3, base_x, base_y, width_ext, fp, DHT_r_DC_C, DHT_r_AC_C);
                }
        }
    flush_BUFFER(fp);
    set_skip_fl(fp, FL_NSKIP);
}

// Processing header
void write_DQT (DQT* obj, u8 id, F_BUFFER* fp) {
    // Write duantizer table

    write_header(M_DQT, fp);
    write_u16(67, fp);                                  // length
    write_u8((obj->precise << 4) + (id & 0x0F), fp);    // precise and id
    for (u8 i = 0; i < N_QUANT; ++i)
        if (obj->precise == 0)
            write_u8((u8)obj->quantizer[i], fp);
        else
            write_u16(obj->quantizer[i], fp);
}

void write_DHT (pre_DHT* obj, u8 id, F_BUFFER* fp) {
    // Write DHT

    write_header(M_DHT, fp);
    write_u16(obj->num_symbol + 19, fp);    // length
    write_u8(id, fp);
    for (u16 i = 0; i < 16; ++i)
        write_u8(obj->num[i], fp);
    for (u16 i = 0; i < obj->num_symbol; ++i)
        write_u8(obj->symbol[i], fp);
}

void write_JPEG_INFO(char* jpeg_path, RGB* img, u8 Y_v_rate, u8 Y_h_rate, u8 C_v_rate, u8 C_h_rate) {
    // Write JPEG Header

    F_BUFFER* fp = init_F_BUFFER(jpeg_path);

    // Write SOI
    write_header(M_SOI, fp);

    // Write APP0
    write_header(M_APP0, fp);
    write_u16(0x10, fp);    // length
    write_u8(0x4A, fp);     // magic number
    write_u8(0x46, fp);
    write_u8(0x49, fp);
    write_u8(0x46, fp);
    write_u8(0x00, fp);
    write_u16(0x0101, fp);  // Ver
    write_u8(0x00, fp);     // NO unit
    write_u16(0x01, fp);    // Pixel density X
    write_u16(0x01, fp);    // Pixel density Y
    write_u8(0x00, fp);     // thumb X
    write_u8(0x00, fp);     // thumb Y

    // Write DQT
    write_DQT(&DQT_Y, DQT_Y_ID, fp);
    write_DQT(&DQT_C, DQT_C_ID, fp);

    // Write SOF0
    write_header(M_SOF0, fp);
    write_u16(17, fp);                          // length
    write_u8(0x08, fp);                         // precise
    write_u16(img->height, fp);                 // length
    write_u16(img->width, fp);                  // length
    write_u8(0x03, fp);                         // # channel
    write_u8(0x01, fp);                         // Color info Y
    write_u8((Y_h_rate << 4) + Y_v_rate, fp);   // Sample rate
    write_u8(DQT_Y_ID, fp);                     // DQT ID
    write_u8(0x02, fp);                         // Color info Cb
    write_u8((C_h_rate << 4) + C_v_rate, fp);   // Sample rate
    write_u8(DQT_C_ID, fp);                     // DQT ID
    write_u8(0x03, fp);                         // Color info Cr
    write_u8((C_h_rate << 4) + C_v_rate, fp);   // Sample rate
    write_u8(DQT_C_ID, fp);                     // DQT ID

    // Write DHT
    write_DHT(&pre_DHT_DC_Y, (TYPE_DC << 4) + DHT_Y_ID, fp);
    write_DHT(&pre_DHT_DC_C, (TYPE_DC << 4) + DHT_C_ID, fp);
    write_DHT(&pre_DHT_AC_Y, (TYPE_AC << 4) + DHT_Y_ID, fp);
    write_DHT(&pre_DHT_AC_C, (TYPE_AC << 4) + DHT_C_ID, fp);

    // Write SOS
    write_header(M_SOS, fp);
    write_u16(12, fp);
    write_u8(3, fp);                            // # color
    write_u8(1, fp);                            // Color Y
    write_u8((DHT_Y_ID << 4) + DHT_Y_ID, fp);   // DHT id
    write_u8(2, fp);                            // Color Y
    write_u8((DHT_C_ID << 4) + DHT_C_ID, fp);   // DHT id
    write_u8(3, fp);                            // Color Y
    write_u8((DHT_C_ID << 4) + DHT_C_ID, fp);   // DHT id
    write_u8(0x00, fp);                         // magic number
    write_u8(0x3F, fp);                         // magic number
    write_u8(0x00, fp);                         // magic number

    // Write DATA
    write_JPEG_data(img, Y_v_rate, Y_h_rate, C_v_rate, C_h_rate, fp);

    // Write EOI
    write_u8(0x00, fp);                         // flush data ?
    write_header(M_EOI, fp);
}

int main (int argc, char* argv[]) {

    // Check command line parameter
    if (argc !=3) {
        printf("Usage\n\t %s bmp_path jpeg_path\n", argv[0]);
        exit(1);
    }

    // Read File
    RGB* img = read_bmp(argv[1]);

    // Constant mode
    u8 Y_v_rate = 1;
    u8 Y_h_rate = 1;
    u8 C_v_rate = 1;
    u8 C_h_rate = 1;

    write_JPEG_INFO(argv[2], img,Y_v_rate, Y_h_rate, C_v_rate, C_h_rate);
    /*write_ppm(img);*/

    return 0;
}
