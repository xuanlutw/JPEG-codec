#include <cstdio>
#include "rgb.h"

RGB::RGB (Picture* pic) {
    this->v_size = pic->v_size;
    this->h_size = pic->h_size;
    this->R = new u8[this->v_size * this->h_size]();
    this->G = new u8[this->v_size * this->h_size]();
    this->B = new u8[this->v_size * this->h_size]();

    for (u16 i = 0; i < this->v_size; ++i)
        for (u16 j = 0; j < this->h_size; ++j) {
            u32 index      = idx2(i, j, this->h_size);
            u32 index_y    = idx2(i * pic->v_fact[1] / pic->mcu_v_size, \
                                  j * pic->h_fact[1] / pic->mcu_h_size, \
                                  pic->h_size_ext * pic->h_fact[1] / pic->mcu_h_size);
            u32 index_cb   = idx2(i * pic->v_fact[2] / pic->mcu_v_size, \
                                  j * pic->h_fact[2] / pic->mcu_h_size, \
                                  pic->h_size_ext * pic->h_fact[2] / pic->mcu_h_size);
            u32 index_cr   = idx2(i * pic->v_fact[3] / pic->mcu_v_size, \
                                  j * pic->h_fact[3] / pic->mcu_h_size, \
                                  pic->h_size_ext * pic->h_fact[3] / pic->mcu_h_size);
            i16 Y          = pic->pix[1][index_y];
            i16 Cb         = pic->pix[2][index_cb];
            i16 Cr         = pic->pix[3][index_cr];
            this->R[index] = bandpass(1.0 * Y                + 1.402   * Cr + 128.0);
            this->G[index] = bandpass(1.0 * Y - 0.34414 * Cb - 0.71414 * Cr + 128.0);
            this->B[index] = bandpass(1.0 * Y + 1.772   * Cb                + 128.0);
        }
}

void RGB::dump_ppm (char* filename) {
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "P3\n%d %d\n255\n", this->h_size, this->v_size);
    for (u16 i = 0; i < this->v_size; ++i) {
        for (u16 j = 0; j < this->h_size; ++j) {
            u32 index = idx2(i, j, this->h_size);
            fprintf(fp, "%d %d %d ", R[index], G[index], B[index]);
        }
        fprintf(fp, "\n");
    }
}

void RGB::dump_bmp (char* filename) {
    FILE* fp = fopen(filename, "w");

    // Image data size with padding
    u32 padding_size = ((this->h_size * 3 + 3) / 4) * 4 - this->h_size * 3;
    u32 img_size     = this->v_size * (this->h_size * 3 + padding_size);

    // File Header
    fprintf(fp, "BM");              // Magic number
    write_u32(fp, 54 + img_size);   // File size, Header size = 54
    write_u32(fp, 0);               // Riverse 1 & 2
    write_u32(fp, 54);              // Offset

    // Info header
    write_u32(fp, 40);              // Info header size
    write_u32(fp, this->h_size);    // Width
    write_u32(fp, this->v_size);    // Height
    write_u16(fp, 1);               // # planes
    write_u16(fp, 24);              // # bits per pixel
    write_u32(fp, 0);               // NO compression
    write_u32(fp, img_size);        // Image size
    write_u32(fp, 100);             // Horizontal resolution
    write_u32(fp, 100);             // Vertical resolution
    write_u32(fp, 0);               // Number of color palette
    write_u32(fp, 0);               // Nnumber of important colors

    // Write pixel data
    for (i32 i = this->v_size - 1; i >= 0; --i) {
        for (u32 j = 0; j < this->h_size; ++j) {
            u32 index = idx2(i, j, this->h_size);
            fprintf(fp, "%c%c%c", this->B[index], this->G[index], this->R[index]);
        }
        for (u16 j = 0; j < padding_size; ++j)
            fprintf(fp, "%c", 0);
    }
    fflush(fp);
    fclose(fp);

    return;
}
