#include <cstdio>
#include <cstdlib>
#include "stream.h"

Stream::Stream(char* file_name) {
    this->fp      = fopen(file_name, "rb");
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

void Stream::align () {
    this->read(this->buf_len % 8);
    check(this->buf_len % 8 == 0, "Stream align fail");
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

void Stream::put (u32 content, u8 len) {
    this->buf     += content << this->buf_len;
    this->buf_len += len;
    check(this->buf_len < 64, "Stream buffer overflow");
}

u8 Stream::next_marker () {
    bool status = false;
    u8   tmp;

    this->align();
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
