#include <stddef.h>

void *memset(void *src, int v, size_t l){
    unsigned char *b = src;
    for(int i = 0; i < l; i++){
        b[i] = v;
    }
    return b;
}