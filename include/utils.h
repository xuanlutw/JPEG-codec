#pragma once
#include <cstdint>

// Define data type
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define check(cond, msg, ...) do {                                             \
                                  if (!(cond)) {                               \
                                      fprintf(stderr, msg"\n", ##__VA_ARGS__); \
                                      exit(-1);                                \
                                  }                                            \
                              } while(0);

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

# define TYPE_R 0
# define TYPE_W 1

# define FL_SKIP 1
# define FL_NSKIP 0

#define idx(i, j) (8 * (i) + (j))
#define idx2(i, j, x) ((u32)(x) * (i) + (j))
#define coef(x, y) (0.5 / cos((x) * M_PI / (y)));

extern u8 zz[64];
u8 bandpass (double val);
void write_u32 (FILE* fp, u32 num);
void write_u16 (FILE* fp, u16 num);
