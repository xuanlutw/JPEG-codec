#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cmath>
#include "utils.h"
#include "stream.h"
#include "huffman.h"
#include "header.h"
#include "picture.h"
#include "rgb.h"

int main (int argc, char* argv[]) {
    if (argc !=3) {
        printf("Usage\n\t %s jpeg_path new_jpeg_path\n", argv[0]);
        exit(1);
    }

    Stream* vs     = new Stream(argv[1], TYPE_R);
    Stream* os;
    DQT* Q         = new DQT();
    DHT* H         = new DHT();
    Picture* pic   = new Picture(H, Q);
    Header* header = NULL;
    u8 marker;

    // Scan and get opt Huffman code
    printf("First Scan\n");
    while (true) {
        marker = vs->next_marker();
        switch (marker) {
            case M_SOI:
                break;
            case M_EOI:
                goto Finish;
            case M_APP0:
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
            case M_COM:
            case M_DQT:
            case M_DRI:
                header = new Header(vs, marker);
                delete header;
                break;
            case M_DHT:
                //printf("DHT\n");
                H->read(vs);
                break;
            case M_SOF0:
                //printf("SOF0\n");
                pic->read_SOF0(vs);
                break;
            case M_SOS:
                //printf("SOS\n");
                pic->read_SOS(vs);
                pic->decode(vs);
                break;
            case 0:
            default:
                printf("Unknow marker at %x\n", marker);
        }
    }

Finish:
    // Compute optimal huffman code
    printf("Compute Optimal Huffman Code\n");
    for (u8 i = 0; i < N_DHT; ++i) {
        if (H->dc[i]) {
            H->dc[i]->optimal();
            //H->dc[i]->triverse();
        }
        if (H->ac[i]) {
            H->ac[i]->optimal();
            //H->ac[i]->triverse();
        }
    }

    // Scan and write opt image
    printf("Second Scan\n");
    delete vs;
    vs = new Stream(argv[1], TYPE_R);
    os = new Stream(argv[2], TYPE_W);
    while (true) {
        marker = vs->next_marker();
        switch (marker) {
            case M_SOI:
                os->write_marker(M_SOI);
                //printf("SOI\n");
                break;
            case M_EOI:
                os->write_marker(M_EOI);
                //printf("EOI\n");
                goto Finish2;
            case M_APP0:
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
            case M_COM:
            case M_DQT:
            case M_DRI:
            case M_SOF0:
                //printf("HEADER\n");
                header = new Header(vs, marker);
                header->write(os);
                delete header;
                break;
            case M_DHT:
                break;
            case M_SOS:
                H->write(os);
                //printf("SOS\n");
                header = new Header(vs, marker);
                header->write(os);
                pic->trans(vs, os);
                break;
            case 0:
            default:
                printf("Unknow marker at %x\n", marker);
        }
    }
Finish2:
    printf("Done\n");
    u32 size_raw = pic->v_size * pic->h_size * 3;
    u32 size_ori = vs->get_size();
    u32 size_red = os->get_size();
    printf("Raw\tOrigin\tReduce\tRate\tRate'\n");
    printf("(kb)\t(kb)\t(kb)\t(%%)\t(%%)\n");
    printf("%.1lf\t%.1lf\t%.1lf\t%.1lf\t%.1lf\n", (double)size_raw / 1024, \
                                         (double)size_ori / 1024, \
                                         (double)size_red / 1024, \
                                         (double)size_ori / size_raw * 100 , \
                                         (double)size_red / size_raw * 100);
}
