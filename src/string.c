#include <stddef.h>
#include <stdint.h>

#define HEXDUMP_COL  16
#define HEXDUMP_LINE (6 + 3*HEXDUMP_COL + 2)

static const char hex_digits[] = "0123456789ABCDEF";

void hex8(char* out, uint8_t b) {
    out[0] = hex_digits[(b >> 4) & 0xF];
    out[1] = hex_digits[b & 0xF];
}

void hex32(char* out, uint32_t b){
    out[0] = '0';
    out[1] = 'x';

    for(int i = 0; i < 4; i++){
        hex8(&out[2+2*i], (b >> 8*(3-i)) & 0xFF);
    }
}

void *memset(void *src, int v, size_t l){
    unsigned char *b = src;
    for(unsigned int i = 0; i < l; i++){
        b[i] = v;
    }
    return b;
}

void *memcpy (void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

int strncmp(const char *s1, const char *s2, register size_t n)
{
    register unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

int strlen(const char *str){
    const char *p;
    if (str == 0)
        return 0;
    p = str;
    while (*p != '\0')  
        p++; 
    return p - str;
}

/// Returns input bytes consumed, set out_written to the number of bytes written to the output buffer.
unsigned int hexdump(const uint8_t *buf, unsigned len, uint8_t *out_buf, unsigned outbuf_len, unsigned *out_written, int base_offset){
    char line[HEXDUMP_LINE] = {0};
    int written = 0;
    *out_written = 0;

    for(unsigned int off = 0; off < len; off+=HEXDUMP_COL) {
        // Number of byte to print in this line
        int n = len-off;
        if(n > HEXDUMP_COL) n = HEXDUMP_COL;
        // If we are about to overshoot the outbuf, return bytes read.
        if(written+6+n*3+2 > outbuf_len){
            return off;
        }
        // Offset: '00EF: '
        hex8(line, ((off+base_offset) >> 8) & 0xFF);
        hex8(&line[2], (off+base_offset) & 0xFF);
        line[4] = ':';
        line[5] = ' ';


        for(int i = 0; i < n; i++){
            hex8(&line[6+i*3], buf[off+i]);
            line[6+i*3+2] = ' ';
        }

        line[6+n*3] = '\r';
        line[6+n*3+1] = '\n';
        memcpy(out_buf, line, 6+n*3+2);
        out_buf+=n*3+8;
        written+=n*3+8;
        *out_written = written;
    }
    return len;
}

char *utoa_back(unsigned long v, char *end, unsigned *len) {
    char *p = end;
    do { *--p = (char)('0' + (v % 10UL)); v /= 10UL; } while (v);
    *len = (unsigned)(end - p);
    return p;
}