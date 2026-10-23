#pragma once

#include <stdint.h>

// Data lives in caller side.
typedef struct{
    const uint8_t *ptr;
    uint8_t remaining;
} ad_iter_t;

typedef struct{
    uint8_t type;
    uint8_t length;
    const uint8_t *value;
} ad_item_t;

void ad_iter_init(ad_iter_t *it, const uint8_t *data, unsigned len);
int ad_iter_next(ad_iter_t *it, ad_item_t *out);