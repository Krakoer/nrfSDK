#pragma once

#include <stddef.h>

void *memcpy (void *dest, const void *src, size_t len);
void *memset(void *, int, size_t);
int strncmp(const char *s1, const char *s2, register size_t n);
void hex8(char* out, uint8_t b);
void hex32(char* out, uint32_t b);
int strlen(const char *str);
unsigned int hexdump(const uint8_t *buf, unsigned len, uint8_t *out_buf, unsigned outbuf_len, unsigned *out_written);
char *utoa_back(unsigned long v, char *end, unsigned *len);