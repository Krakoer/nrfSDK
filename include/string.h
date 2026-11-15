#pragma once

#include <stddef.h>

void *memcpy (void *dest, const void *src, size_t len);
void *memset(void *, int, size_t);
int strncmp(const char *s1, const char *s2, register size_t n);