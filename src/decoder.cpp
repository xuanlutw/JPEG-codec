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
    while (true) {
        marker = vs->next_marker();
        switch (marker) {
            case M_SOI:
                printf("SOI\n");
                break;
            case M_EOI:
                printf("EOI\n");
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
                printf("HEADER\n");
                break;
            case M_DHT:
                printf("DHT\n");
                H->read(vs);
                break;
            case M_SOF0:
                printf("SOF0\n");
                pic->read_SOF0(vs);
                break;
            case M_SOS:
                printf("SOS\n");
                pic->read_SOS(vs);
                pic->decode(vs);
                break;
            case 0:
            default:
                printf("Unknow marker at %x\n", marker);
        }
    }

    // Compute optimal huffman code
Finish:
    printf("FUIYOU\n");
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
    delete vs;
    vs = new Stream(argv[1], TYPE_R);
    os = new Stream(argv[2], TYPE_W);
    while (true) {
        marker = vs->next_marker();
        switch (marker) {
            case M_SOI:
                os->write_marker(M_SOI);
                printf("SOI\n");
                break;
            case M_EOI:
                os->write_marker(M_EOI);
                printf("EOI\n");
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
                printf("HEADER\n");
                header = new Header(vs, marker);
                header->write(os);
                delete header;
                break;
            case M_DHT:
                break;
            case M_SOS:
                H->write(os);
                printf("SOS\n");
                header = new Header(vs, marker);
                header->write(os);
                pic->trans(vs, os);
                break;
            case 0:
            default:
                printf("Unknow marker at %x\n", marker);
        }
    }
    //RGB rgb(pic);
    //rgb.dump_bmp(argv[2]);
Finish2:
    printf("FUIYOU\n");
    //for (u8 i = 0; i < N_DHT; ++i) {
        //if (H->dc[i]) {
            //H->dc[i]->optimal();
            ////H->dc[i]->triverse();
        //}
        //if (H->ac[i]) {
            //H->ac[i]->optimal();
            ////H->ac[i]->triverse();
        //}
    //}
}
