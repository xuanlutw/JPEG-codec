#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "block.h"

Block::Block (Stream* vs, Huffman* DC, Huffman* AC, u16* q, i16* dc_pred) {
    for (u8 i = 0; i < 64; ++i)
        data[i] = 0;

    data[0] = (*dc_pred = *dc_pred + DC->get(vs));
    for (u8 i = 1; i < 64; ++i) {
        AC->get(vs);
        if (AC->R == 0x0 && AC->S == 0x0)
            break;
        i += AC->R;
        data[i] = AC->S;
    }
    //this->print();

    this->inverse_q(q);
    //this->print();

    this->inverse_scan();
    //this->print();

    this->inverse_DCT();
    //this->print();
}

void Block::inverse_scan () {
    i16 tmp[64];
    memcpy(tmp, this->data, sizeof(i16) * 64);
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            this->data[idx(i, j)] = tmp[zz[idx(i, j)]];
}

void Block::inverse_q (u16* q) {
    for (u8 i = 0; i < N_QUANT; ++i)
        this->data[i] *= q[i];
}

void Block::inverse_DCT1 () {
    static double* cos_table = NULL;
    static double* coeff     = NULL;

    // Init cos table and only once
    if (!cos_table) {
        cos_table = (double*)malloc(sizeof(double) * M_COS);
        for (u8 i = 0; i < M_COS; i++)
            cos_table[i] = cos(i * M_PI / 16.0);
    }

    // Init coefficient and only once
    if (!coeff) {
        coeff = (double*)malloc(sizeof(double) * 8);
        coeff[0] = 1. / sqrt(2.);
        for (u8 i = 1; i < 8; i++)
            coeff[i] = 1.;
    }

    // Transform
    i16 tmp[64] = {0};
    for (int j = 0; j < 8; j++)
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++)
                tmp[idx(j, x)] += coeff[y] * this->data[idx(x, y)] * cos_table[((j << 1) + 1) * y];
            tmp[idx(j, x)] /= 2.;
        }
    memcpy(this->data, tmp, sizeof(i16) * 64);
}

void Block::inverse_DCT1_fast () {
    static double C1_4  = coef(1., 4.);
    static double C1_8  = coef(1., 8.);
    static double C3_8  = coef(3., 8.);
    static double C1_16 = coef(1., 16.);
    static double C3_16 = coef(3., 16.);
    static double C5_16 = coef(5., 16.);
    static double C7_16 = coef(7., 16.);
    double f[8];
    double g[8];
    double tmp;
    i16    tmp_data[64];

    for (u8 i = 0; i < 8; ++i) {
        // Stage 1
        f[0] = this->data[idx(i, 0)] / M_SQRT2;
        f[1] = this->data[idx(i, 4)];
        f[2] = this->data[idx(i, 2)];
        f[3] = this->data[idx(i, 6)];
        f[4] = this->data[idx(i, 1)];
        f[5] = this->data[idx(i, 5)] + this->data[idx(i, 3)];
        f[6] = this->data[idx(i, 3)] + this->data[idx(i, 1)];
        f[7] = this->data[idx(i, 7)] + this->data[idx(i, 5)];

        // Stage 2
        f[3] += f[2];
        f[7] += f[6];

        // Stage 3
        f[1] *= C1_4;
        f[3] *= C1_4;
        f[5] *= C1_4;
        f[7] *= C1_4;

        // Stage 4
        tmp  = f[0];
        f[0] = tmp + f[1];
        f[1] = tmp - f[1];
        tmp  = f[2];
        f[2] = tmp + f[3];
        f[3] = tmp - f[3];
        tmp  = f[4];
        f[4] = tmp + f[5];
        f[5] = tmp - f[5];
        tmp  = f[6];
        f[6] = tmp + f[7];
        f[7] = tmp - f[7];

        // Stage 5
        f[2] *= C1_8;
        f[3] *= C3_8;
        f[6] *= C1_8;
        f[7] *= C3_8;

        // Stage 6
        g[0] = f[0] + f[2];
        g[1] = f[1] + f[3];
        g[2] = f[0] - f[2];
        g[3] = f[1] - f[3];
        g[4] = f[4] + f[6];
        g[5] = f[5] + f[7];
        g[6] = f[4] - f[6];
        g[7] = f[5] - f[7];

        // Stage 7
        g[4] *= C1_16;
        g[5] *= C3_16;
        g[6] *= C7_16;
        g[7] *= C5_16;

        // Stage 8
        tmp_data[idx(0, i)] = round(g[0] + g[4]);
        tmp_data[idx(1, i)] = round(g[1] + g[5]);
        tmp_data[idx(3, i)] = round(g[2] + g[6]);
        tmp_data[idx(2, i)] = round(g[3] + g[7]);
        tmp_data[idx(7, i)] = round(g[0] - g[4]);
        tmp_data[idx(6, i)] = round(g[1] - g[5]);
        tmp_data[idx(4, i)] = round(g[2] - g[6]);
        tmp_data[idx(5, i)] = round(g[3] - g[7]);
    }
    memcpy(this->data, tmp_data, sizeof(i16) * 64);
}

void Block::inverse_DCT () {
    this->inverse_DCT1_fast();
    this->inverse_DCT1_fast();
    for (u8 i = 0; i < 64; ++i)
        this->data[i] >>= 2;
    //this->inverse_DCT1();
    //this->inverse_DCT1();
}

void Block::print () {
    for (int i = 0; i < 8; ++i) {
        printf("  ");
        for (int j = 0; j < 8; ++j)
            printf("%3d ", data[idx(i, j)]);
        printf("\n");
    }
    printf("\n");
}
