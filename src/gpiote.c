#include "gpiote.h"

void gpiote_configure(int n, gpiote_config_t *cfg){
    GPIOTE_CONFIG_N(n) =   (cfg->mode & 0x3UL)
          | ((cfg->pin.pin & 0x1FUL)             << 8)
          | (((cfg->pin.base & 0x300UL) >> 9)    << 13)
          | ((cfg->polarity & 0x3UL)             << 16)
          | ((cfg->outinit & 0x1UL)              << 20);

}