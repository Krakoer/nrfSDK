#include <stddef.h>

void *memset(void *src, int v, size_t l){
    unsigned char *b = src;
    for(unsigned int i = 0; i < l; i++){
        b[i] = v;
    }
    return b;
}

int strncmp(const char *s1, const char *s2, register size_t n)
{
  register unsigned char u1, u2;

  while (n-- > 0)
    {
      u1 = (unsigned char) *s1++;
      u2 = (unsigned char) *s2++;
      if (u1 != u2)
	return u1 - u2;
      if (u1 == '\0')
	return 0;
    }
  return 0;
}