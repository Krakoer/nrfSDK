#include "ble_ad.h"

void ad_iter_init(ad_iter_t *it, const uint8_t *data, unsigned len){
    it->ptr = data;
    it->remaining = len;
}

/// Returns the number of bytes parsed.
int ad_iter_next(ad_iter_t *it, ad_item_t *out){
    // We need at least to bytes for a valid LTU.
    // This way, out->value = it->ptr+2 doesn't go out of bound.
    if(it->remaining < 2){
        return 0;
    }

    // Avoid length read underflow
    if(it->ptr[0] == 0){
        return 0;
    }

    // Check that length doesn't make us go out
    // of bound in next read, when we set it->ptr
    if(it->ptr[0] > it->remaining-1){
        return 0;
    }

    out->length = it->ptr[0]-1;
    out->type = it->ptr[1];
    out->value = it->ptr+2;

    it->ptr += it->ptr[0]+1;
    it->remaining -= out->length+2;
    
    return out->length+2;
}