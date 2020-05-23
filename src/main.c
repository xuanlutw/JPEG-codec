# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <math.h>
# include <unistd.h>

# define M_HEAD 0xFF
# define M_SOI  0xD8
# define M_APP0 0xE0
# define M_DQT  0xDB
# define M_SOF0 0xC0
# define M_DHT  0xC4
# define M_DRI  0xDD
# define M_SOS  0xDA
# define M_EOI  0xD9

# define N_DQT   16
# define N_QUANT 64
# define N_CHAN  5
# define N_DHT   16
# define N_HDEP  16

# define N_BLOCK 10

# define TYPE_DC 0
# define TYPE_AC 1

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;

typedef i8 BLOCK[N_QUANT];

typedef struct {
    BLOCK* BLOCK[N_CHAN][N_BLOCK];
} MCU;

typedef struct {
    u32 buffer;
    u8 len;
    u8 skip_fl;
    FILE* fp;
} F_BUFFER;

u8 read_BUFFER (u16* content, u8 len, F_BUFFER* buf) {
    if (len > 16) {
        printf("len less equal 16!\n");
        printf("%d\n", len);
        exit(1);
    }
    if (buf->len >= len) {
        *content = (u16)(buf->buffer >> (buf->len - len));
        buf->buffer &= ((1 << (buf->len - len)) - 1);
        buf->len -= len;
        return 1;
    }
    else {
        u8 c;
        u8 ret = fread(&c, sizeof(u8), 1, buf->fp);
        if (ret == 0) 
            return 0;
        buf->buffer <<= 8;
        buf->buffer += c;
        buf->len += 8;
        if (buf->skip_fl && c == 0xFF) {
            fread(&c, sizeof(u8), 1, buf->fp);
            if (c != 0x00) {
                printf("0xFF coding error\n");
                exit(1);
            }
        }
        return read_BUFFER(content, len, buf);
    }
}

void put_BUFFER (u8 content, u8 len, F_BUFFER* buf) {
    buf->buffer += content << buf->len;
    buf->len += len;
}

u8 read_u8 (u8* content, F_BUFFER* buf) {
    u16 content16;
    u8 ret = read_BUFFER(&content16, 8, buf);
    *content = (u8)(content16);
    return ret;
}

u8 read_u16 (u16* content, F_BUFFER* buf) {
    return read_BUFFER(content, 16, buf);
    /*u8 ret = read_BUFFER(content, 16, buf);*/
    /**content = (*content << 8) + (*content >> 8);*/
    /*return ret;*/
}

F_BUFFER* init_F_BUFFER (char* file_path) {
    F_BUFFER* obj = malloc(sizeof(F_BUFFER));
    obj->fp = fopen(file_path, "rb");
    obj->skip_fl = 0;
    obj->len = 0;
    obj->buffer = 0;
    return obj;
}

typedef struct DQT_ {
    u8 precise;
    u16 quantizer[N_QUANT];
} DQT;

typedef struct CHAN_ {
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

typedef struct DHT_ {
    H_NODE* root;
    H_NODE* pt;
    u8 content;
    u8 len;
    u8 symbol;
    u8 status;
} DHT;

u8 H_NODE_is_leaf (H_NODE* root) {
    return (root->l == root);
}

u8 H_NODE_symbol (H_NODE* root) {
    return (u8)root->r;
}

void H_NODE_add_symbol (H_NODE* root, u16 content, u8 len, u8 symbol) {
    if (root->l == root) {
        printf("Wrong tree construction!\n");
        exit(1);
    }
    if (len == 0) {
        root->l = root;
        root->r = (H_NODE*)(u64)symbol;
    }
    else {
        if ((content >> (len - 1)) & 1) {
            if (root->r == NULL) {
                H_NODE* obj = malloc(sizeof(H_NODE));
                obj->l = NULL;
                obj->r = NULL;
                root->r = obj;
            }
            H_NODE_add_symbol(root->r, content, len - 1, symbol);
        }
        else {
            if (root->l == NULL) {
                H_NODE* obj = malloc(sizeof(H_NODE));
                obj->l = NULL;
                obj->r = NULL;
                root->l = obj;
            }
            H_NODE_add_symbol(root->l, content, len - 1, symbol);
        }
    }
}

void H_NODE_triverse (H_NODE* root, u16 content) {
    if (root->l == root) {
        printf("%x %d\n", content, (u8)root->r);
    }
    else {
        if (root->l)
            H_NODE_triverse(root->l, content << 1);
        else 
            printf("NULLLLLLLLLLLLLLLL\n");
        if (root->r)
            H_NODE_triverse(root->r, (content << 1) + 1);
        else 
            printf("NULLLLLLLLLLLLLLLL\n");
    }
}

DHT* init_DHT () {
    DHT* obj = malloc(sizeof(DHT));
    H_NODE* root = malloc(sizeof(H_NODE));
    obj->root = root;
    root->r = NULL;
    root->l = NULL;
    return obj;
}

void DHT_reset(DHT* table) {
    table->len = 0;
    table->pt = table->root;
    table->status = 0;
}

u8 DHT_run(DHT* table) {
    if (H_NODE_is_leaf(table->pt)) {
        table->symbol = H_NODE_symbol(table->pt);
        table->pt = table->root;
        table->status = 1;
        return 1;
    }
    if (table->len == 0) {
        table->status = 0;
        return 0;
    }
    if (((table->content) >> (table->len - 1)) & 1)
        table->pt = table->pt->r;
    else
        table->pt = table->pt->l;
    table->len--;
    return DHT_run(table);
}

u8 DHT_put(DHT* table, u8 content, u8 len) {
    table->content = content;
    table->len = len;
    return DHT_run(table);
}

u8 DHT_get_symbol (DHT* table) {
    return table->symbol;
}

u8 DHT_get_content (DHT* table) {
    return table->content;
}
u8 DHT_get_len (DHT* table) {
    return table->len;
}

u8 DHT_get_status (DHT* table) {
    return table->status;
}

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
} JPEG_INFO;

JPEG_INFO* init_JPEG_INFO (char* jpeg_path) {
    JPEG_INFO* obj = malloc(sizeof(JPEG_INFO));
    obj->fp = init_F_BUFFER(jpeg_path);
    obj->DRI = 0;
    obj->MCU_v_factor = 0;
    obj->MCU_h_factor = 0;
    for (int i = 0;i < N_DQT;++i)
        obj->DQT[i] = NULL;
    for (int i = 0;i < N_CHAN;++i)
        obj->CHAN[i] = NULL;
    return obj;
}
    
void read_APP0 (JPEG_INFO* info) {
    u16 len;
    read_u16(&len, info->fp);
    u8 tmp[len - 2];
    for (int i = 0; i < len - 2; ++i) {
        u8 c;
        read_u8(&c, info->fp);
    }
    /*fread(tmp, 1, len - 2, info->fp);*/
    return;
    
    // Skip
    u8 tag[5];
    u8 ver[2];
    u8 unit[1];
    u8 x_dens[2];
    u8 y_dens[2];
    u8 thumb_x[2];
    u8 thumb_y[2];

    fread(tag, 1, 5, info->fp);
    fread(ver, 1, 2, info->fp);
    printf("%c%c%c%c%c\n", tag[0], tag[1], tag[2], tag[3], tag[4]);
}
    
void read_DQT (JPEG_INFO* info) {
    u16 len;
    u8 repeat; 
    read_u16(&len, info->fp);
    repeat = (len - 2) / (N_QUANT + 1);
    if (repeat * (N_QUANT + 1) + 2 != len) {
        printf("Wrong DQT size");
        exit(1);
    }

    for (int i = 0; i < repeat; ++i) {
        DQT* obj = malloc(sizeof(DQT));
        u8 val;
        read_u8(&val, info->fp);
        obj->precise = val >> 4;
        info->DQT[val & 0x0F] = obj;
        for (int i = 0;i < N_QUANT;++i) 
            if (obj->precise == 0)
                read_u8((u8*)(obj->quantizer + i), info->fp);
            else
                // TODO
                read_u16(obj->quantizer + i, info->fp);
    }
}

void read_SOF0 (JPEG_INFO* info) {
    u16 len;
    read_u16(&len, info->fp);
    read_u8(&(info->precise), info->fp);
    read_u16(&(info->height), info->fp);
    read_u16(&(info->width), info->fp);
    read_u8(&(info->n_color), info->fp);

    if (8 + 3 * info->n_color != len) {
        printf("Wrong SOF0 size");
        exit(1);
    }

    for (int i = 0; i < info->n_color; ++i) {
        CHAN* obj = malloc(sizeof(CHAN));
        u8 id;
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

    info->MCU_NV = info->height / (info->MCU_v_factor * 8) + ((info->height % (info->MCU_v_factor * 8))? 1: 0);
    info->MCU_NH = info->width  / (info->MCU_h_factor * 8) + ((info->width  % (info->MCU_h_factor * 8))? 1: 0);
    printf("gCU = %d %d\n", info->MCU_NV, info->MCU_NH);
    /*printf("%d\n", (info->height));*/
    /*printf("%d\n", (info->width));*/
    /*printf("%d\n", (info->n_color));*/
}

void read_DHT (JPEG_INFO* info) {
    u16 len;
    read_u16(&len, info->fp);
    len -= 2;

    while (1) {
        if (len == 0)
            break;
        else if (len & 0x8000) {
            printf("DHT len error!\n");
            exit(1);
        }
        DHT* obj = init_DHT();
        u16 counter = 0;
        u8 c_len = 0xFF;
        u16 content;
        u8 symbol;
        u8 num[N_HDEP];
        u8 c;
        
        read_u8(&c, info->fp);
        if (c >> 4 == TYPE_DC)
            info->DC_DHT[c & 0x0F] = obj;
        else if (c >> 4 == TYPE_AC)
            info->AC_DHT[c & 0x0F] = obj;
            
        for (int i = 0; i < N_HDEP; ++i) {
            read_u8(num + i, info->fp);
            /*printf(">> %d\n", num[i]);*/
            counter += num[i];
            if (num[i] != 0 && c_len == 0xFF)
                c_len = i;
        }
        
        content = 0;
        for (int i = 0; i < counter; ++i) {
            read_u8(&symbol, info->fp);
            /*printf("%d %d %d %d\n", i, content, c_len, num[c_len]);*/
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
    u16 len;
    read_u16(&len, info->fp);
    if (len != 4) {
        printf("DRI len error!\n");
        exit(1);
    }
    read_u16(&(info->DRI), info->fp);
}

void read_SOS (JPEG_INFO* info) {
    u16 len;
    read_u16(&len, info->fp);
    len -= 2;

    read_u8(&(info->n_color), info->fp);
    for (int i = 0; i < (len - 4) / 2; ++i) {
        /*printf(">>%d\n", i);*/
        u8 ID;
        u8 table;
        read_u8(&ID, info->fp);
        read_u8(&table, info->fp);
        info->CHAN[ID]->DC_table = table >> 4;
        info->CHAN[ID]->AC_table = table & 0x0F;
        printf("%d %d\n", ID, table >> 4);
    }
    u8 tmp;
    read_u8(&tmp, info->fp);
    if (tmp != 0x00) {
        printf("Wrong SOS-1\n");
        exit(1);
    }
    read_u8(&tmp, info->fp);
    if (tmp != 0x3F) {
        printf("Wrong SOS-2\n");
        exit(1);
    }
    read_u8(&tmp, info->fp);
    if (tmp != 0x00) {
        printf("Wrong SOS-base\n");
        exit(1);
    }
}

u8 get_symbol_len (DHT* DHT_table, F_BUFFER* buf) {
    DHT_reset(DHT_table);
    while (!DHT_get_status(DHT_table)) {
        u8 c;
        read_u8(&c, buf);
        DHT_put(DHT_table, c, 8);
    }
    u8 remain_content = DHT_get_content(DHT_table);
    u8 remain_len = DHT_get_len(DHT_table);
    put_BUFFER(remain_content, remain_len, buf);
    return DHT_get_symbol(DHT_table);
}

i8 get_symbol (u8 len, F_BUFFER* buf) {
    u16 c;
    read_BUFFER(&c, len, buf);
    if (c >> (len - 1))
        return (i8)c;
    else 
        return -((i8)c);
}

BLOCK* read_BLOCK(JPEG_INFO* info, u8 chan_id) {
    DHT* DC_table = info->DC_DHT[info->CHAN[chan_id]->DC_table];
    DHT* AC_table = info->AC_DHT[info->CHAN[chan_id]->AC_table];
    BLOCK* obj = malloc(sizeof(BLOCK));
    u8 symbol_len;

    // DC
    symbol_len = get_symbol_len(DC_table, info->fp);
    (*obj)[0] = get_symbol(symbol_len, info->fp);
    /*printf(">> DC DONE<<\n");*/

    // AC
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
    /*printf(">>> BLOCK <<<\n");*/
    /*for (int i = 0; i < N_QUANT; ++i)*/
        /*printf("%d\n", (*obj)[i]);*/
    return obj;
}

void anti_zz(BLOCK* obj) {
    const u8 zz_map[N_QUANT] = { \
        0,  1,  5,  6, 14, 15, 27, 28 , \
        2,  4,  7, 13, 16, 26, 29, 42 , \
        3,  8, 12, 17, 25, 30, 41, 43 , \
        9, 11, 18, 24, 31, 40, 44, 53 , \
        10, 19, 23, 32, 39, 45, 52, 54 , \
        20, 22, 33, 38, 46, 51, 55, 60 , \
        21, 34, 37, 47, 50, 56, 59, 61 , \
        35, 36, 48, 49, 57, 58, 62, 63 };
    BLOCK tmp;
    for (int i = 0; i < N_QUANT; ++i) {
        tmp[i] = (*obj)[zz_map[i]];
    }
    memcpy(*obj, tmp, sizeof(i8) * N_QUANT);
}

void anti_q(BLOCK* obj, DQT* quant) {
    for (int i = 0; i < N_QUANT; ++i) {
        (*obj)[i] *= quant->quantizer[i];
    }
}

/***************************************************************************/
/* I DONT KNOW QAQ */
double cos_table[200];

void init_cos_table() {
    for (u8 i = 0; i < 200; i++) {
        cos_table[i] = cos(i * M_PI / 16.0);
    }
}

double c(int i) {
    double x = 1.0/sqrt(2.0);
    if (i == 0) {
        return x;
    } else {
        return 1.0;
    }
}

void iDCT(BLOCK* obj) {
    BLOCK s = {0};
    BLOCK tmp = {0};
    for (int j = 0; j < 8; j++) {
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                s[8 * j + x] += c(y) * (*obj)[8 * x + y] * cos_table[(j + j + 1) * y];
            }
            s[8 * j + x] = s[8 * j + x] / 2.0;
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int x = 0; x < 8; x++) {
                tmp[8 * i + j] += c(x) * s[j * 8 + x] * cos_table[(i + i + 1) * x];
            }
            tmp[i * 8 + j] = tmp[i * 8 + j] / 2.0;
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            (*obj)[i * 8 + j] = tmp[i * 8 + j];
        }
    }
}
/******************************************************************************/

MCU* read_MCU(JPEG_INFO* info) {
    MCU* obj = malloc(sizeof(MCU));
    for (u8 chan = 1; chan <= 3; ++chan) { // TODO
        /*printf("%d %d %d \n", info->CHAN[chan]->h_factor, info->CHAN[chan]->v_factor, info->CHAN[chan]->N_BLOCK_MCU);*/
        for (u8 i = 0; i < info->CHAN[chan]->N_BLOCK_MCU; ++i) {
            /*printf(">>%d %d\n", chan, i);*/
            obj->BLOCK[chan][i] = read_BLOCK(info, chan);
            anti_zz(obj->BLOCK[chan][i]);
        }
    }
    return obj;
}

typedef struct {
    MCU** MCU;
} JPEG_DATA;

JPEG_DATA* read_JPEG_DATA(JPEG_INFO* info) {
    init_cos_table();
    JPEG_DATA* obj = malloc(sizeof(JPEG_DATA));
    u16 N_MCU = info->MCU_NV * info->MCU_NH;
    obj->MCU = malloc(sizeof(MCU*) * N_MCU);
    info->fp->skip_fl = 1;
    for (u16 i = 0; i < N_MCU; ++i) {
        obj->MCU[i] = read_MCU(info);
    }
    for (u8 chan = 1; chan <= 3; ++chan) { // TODO
        i8 pre = 0;
        for (u16 i = 0; i < N_MCU; ++i) {
            for (u8 j = 0; j < info->CHAN[chan]->N_BLOCK_MCU; ++j) {
                (*obj->MCU[i]->BLOCK[chan][j])[0] += pre;
                pre = (*obj->MCU[i]->BLOCK[chan][j])[0];
                anti_q(obj->MCU[i]->BLOCK[chan][j], info->DQT[info->CHAN[chan]->DQT_ID]);
                iDCT(obj->MCU[i]->BLOCK[chan][j]);
            }
        }
    }
    info->fp->skip_fl = 0;
    info->fp->len = (info->fp->len / 8) * 8; //ALIGN
    return obj;
}

void anti_sampling (JPEG_INFO* info, JPEG_DATA* data) {
    u16 N_MCU = info->MCU_NV * info->MCU_NH;
    u8 Y[480][640];
    /*for (u8 i = 0; i < info->MCU_NV; ++i) {*/
        /*for (u8 j = 0; j < info->MCU_NH; ++j) {*/
            /*for (u8 k = 0; k < info->CHAN[1]->v_factor; ++k) {*/
                /*for (u8 l = 0; l < info->CHAN[1]->h_factor; ++l) {*/
                    /*for (u8 m = 0; m < 8; ++m) {*/
                        /*for (u8 n = 0; n < 8; ++n) {*/
    for (u8 i = 0; i < 30; ++i) {
        for (u8 j = 0; j < 40; ++j) {
            for (u8 k = 0; k < 2; ++k) {
                for (u8 l = 0; l < 2; ++l) {
                    for (u8 m = 0; m < 8; ++m) {
                        for (u8 n = 0; n < 8; ++n) {
                            Y[16 * i + 8 * k + m][16 * j + 8 * l + n] = \
                                (*data->MCU[40 * i + j]->BLOCK[1][2 * k + l])[8 * m + n] + 128;
                        }
                    }
                }
            }
        }
    }
    FILE* fp = fopen("test.pgm", "w");
    fprintf(fp, "P2\n640 480\n255\n");
    for (u16 i = 0; i < 480; ++i) {
        for (u16 j = 0; j < 640; ++j) {
            fprintf(fp, "%d ", Y[i][j]);
        }
    fprintf(fp, "\n");
    }
}

void* load_jpeg (char* jpeg_path) {
    JPEG_INFO* info = init_JPEG_INFO(jpeg_path);

    u8 val_now;
    u32 counter = 0;

    /*fseek(info->fp, SEEK_SET, 0);*/
    // Counter is WRONG!!!
    while (read_u8(&val_now, info->fp)) {
        if (val_now == M_HEAD) {
            read_u8(&val_now, info->fp);
            switch (val_now) {
                case M_SOI:
                    printf("SOI at %d\n", counter);
                    break;
                case M_EOI:
                    printf("EOI at %d\n", counter);
                    break;
                case M_APP0:
                    read_APP0(info);
                    printf("APP0 at %d\n", counter);
                    break;
                case M_DQT:
                    read_DQT(info);
                    printf("DQT at %d\n", counter);
                    break;
                case M_SOF0:
                    read_SOF0(info);
                    printf("SOF0 at %d\n", counter);
                    break;
                case M_DHT:
                    read_DHT(info);
                    printf("DHT at %d\n", counter);
                    break;
                case M_DRI:
                    read_DRI(info);
                    printf("DRI at %d\n", counter);
                    break;
                case M_SOS:
                    read_SOS(info);
                    JPEG_DATA* data = read_JPEG_DATA(info);
                    printf(">><<");
                    anti_sampling(info, data);
                    /*read_BLOCK(info, 0);*/
                    break;
                case 0:
                    /*break;*/
                default:
                    /*val_now = 1;*/
                    printf("Unknow marker at %d, %x\n", counter, val_now);
            }
            counter = counter + 2;
        }
        else {
            printf("Unknow byte at %d, %x\n", counter, val_now);
            counter++;
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
    void* jpeg = load_jpeg(argv[1]);
    
}
