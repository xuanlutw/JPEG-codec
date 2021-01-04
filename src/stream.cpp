#include <cstdio>
#include <cstdlib>
#include "stream.h"

Stream::Stream(char* file_name, u8 type) {
    this->type    = type;
    switch (this->type) {
        case TYPE_R:
            this->fp = fopen(file_name, "rb");
            break;
        case TYPE_W:
            this->fp = fopen(file_name, "wb");
            break;
        default:
            check(0, "Stream type error");
            break;
    }
    this->buf     = 0;
    this->buf_len = 0;
    this->skip_fl = FL_NSKIP;
    this->counter = 0;
    check(fp, "Open file %s fail!\n", file_name);
}

Stream::~Stream() {
    fclose(this->fp);
}

bool Stream::read_buf () {
    u8 c;
    if (!fread(&c, sizeof(u8), 1, this->fp))
        return false;
    this->buf    <<= 8;
    this->buf     += c;
    this->buf_len += 8;
    ++counter;
    if (this->skip_fl == FL_SKIP && c == 0xFF)
        if (!fread(&c, sizeof(u8), 1, this->fp))
            return false;
    return true;
}

void Stream::read_align () {
    this->read(this->buf_len % 8);
    check(this->buf_len % 8 == 0, "Stream read align fail");
}

u32 Stream::read (u8 len) {
    check(len < 32, "Length more then 32!");

    while (this->buf_len < len)
        this->read_buf();

    u32 ret        = (this->buf >> (this->buf_len - len)) & ((1 << len) - 1);
    this->buf     &= ((1 << (this->buf_len - len)) - 1);
    this->buf_len -= len;
    return ret;
}

u8 Stream::read_u8 () {
    return this->read(8);
}

u16 Stream::read_u16 () {
    return this->read(16);
}

bool Stream::write_buf () {
    while (this->buf_len >= 8) {
        u8 c = (this->buf >> (this->buf_len - 8)) & 0xFF;
        this->buf_len -= 8;
        if (!fwrite(&c, sizeof(u8), 1, this->fp))
            return false;
        ++counter;
        if (this->skip_fl == FL_SKIP && c == 0xFF) {
            c = 0x00;
            if (!fwrite(&c, sizeof(u8), 1, this->fp))
                return false;
        }
    }
    return true;
}

void Stream::write_align () {
    this->write(0, (8 - this->buf_len % 8) % 8);
    check(this->buf_len % 8 == 0, "Stream read align fail");
}

void Stream::write (u32 content, u8 len) {
    check(len < 32, "Length more then 32!");

    this->write_buf();
    this->buf    <<= len;
    this->buf     += (content & ((1 << len) - 1));
    this->buf_len += len;
    this->write_buf();
}

void Stream::write_u8 (u8 content) {
    return this->write(content, 8);
}

void Stream::write_u16 (u16 content) {
    return this->write(content, 16);
}

void Stream::write_marker (u8 marker) {
    this->write_align();
    this->write_u8(0xFF);
    this->write_u8(marker);
}

void Stream::put (u32 content, u8 len) {
    this->buf     += content << this->buf_len;
    this->buf_len += len;
    check(this->buf_len < 64, "Stream buffer overflow");
}

u8 Stream::next_marker () {
    bool status = false;
    u8   tmp;

    this->read_align();
    while (true) {
        tmp = this->read_u8();
        if (status && tmp != 0xFF)
            break;
        if (tmp == 0xFF)
            status = true;
    }
    this->marker = tmp;
    return tmp;
}

void Stream::set_skip_fl (bool fl) {
    this->skip_fl =fl;
    return;
}
