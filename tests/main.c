#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "../include/ble_ad.h"

// Actual ADV data:
const uint8_t ADV_0[] = {0x02, 0x01, 0x1A, 0x02, 0x08, 0x00, 0x03, 0x03, 0x61, 0xFE, 0x10, 0xFF, 0x03, 0x00, 0x00, 0x60, 0x55, 0x00, 0x00, 0x00, 0x54, 0x08, 0x3B, 0xC1, 0x1D, 0xEA, 0x32};
const uint8_t ADV_1[] = {0x02, 0x01, 0x1A, 0x02, 0x0A, 0x06, 0x0B, 0xFF, 0x4C, 0x00, 0x10, 0x06, 0x71, 0x1D, 0x32, 0x48, 0x1D, 0x68};
const uint8_t ADV_2[] = {0x02, 0x01, 0x02, 0x16, 0x16, 0xF1, 0xFC, 0x04, 0x72, 0xF1, 0x2B, 0x0E, 0xD8, 0xB9, 0x72, 0x71, 0x41, 0x32, 0x82, 0xD4, 0x7C, 0x02, 0x60, 0xDD, 0x3F, 0xC6};

// BAD ADV
// Length > actual buffer
const uint8_t BAD_ADV_0[] = {0x02, 0x00};
// Single length byte
const uint8_t BAD_ADV_1[] = {0x10};
// Invalid length
const uint8_t BAD_ADV_2[] = {0x00, 0x01, 0x02};
// Null terminator
const uint8_t BAD_ADV_3[] = {0x02, 0x01, 0x06, 0x00, 0x02, 0x09, 0x41};
// Invalide length
const uint8_t BAD_ADV_4[] = {0x02, 0x01, 0x06, 0x60, 0x02, 0x09, 0x41};

const uint8_t PARSED_0[] = {0x03, 0x03, 0x04, 0x11};

int main(){
    ad_iter_t iter;
    ad_item_t el;

    ad_iter_init(&iter, ADV_0, sizeof ADV_0/sizeof ADV_0[0]);
    int parsed = 0;
    int sum = 0;
    unsigned i = 0;
    while((parsed = ad_iter_next(&iter, &el))){
        assert(i < sizeof PARSED_0 / sizeof PARSED_0[0]);
        assert(parsed == PARSED_0[i]);
        i++;
        sum += parsed;
    }
    assert(sum == sizeof ADV_0 / sizeof ADV_0[0]);
    assert(i == sizeof PARSED_0 / sizeof PARSED_0[0]);

    ad_iter_init(&iter, BAD_ADV_0, sizeof BAD_ADV_0/sizeof BAD_ADV_0[0]);
    assert(ad_iter_next(&iter, &el) == 0);

    ad_iter_init(&iter, BAD_ADV_1, sizeof BAD_ADV_1/sizeof BAD_ADV_1[0]);
    assert(ad_iter_next(&iter, &el) == 0);

    ad_iter_init(&iter, BAD_ADV_2, sizeof BAD_ADV_2/sizeof BAD_ADV_2[0]);
    assert(ad_iter_next(&iter, &el) == 0);

    ad_iter_init(&iter, BAD_ADV_3, sizeof BAD_ADV_3/sizeof BAD_ADV_3[0]);
    assert(ad_iter_next(&iter, &el) != 0);
    assert(ad_iter_next(&iter, &el) == 0);

    ad_iter_init(&iter, BAD_ADV_4, sizeof BAD_ADV_4/sizeof BAD_ADV_4[0]);
    assert(ad_iter_next(&iter, &el) != 0);
    assert(ad_iter_next(&iter, &el) == 0);

    printf("[+] BLE AD tests OK\n");
}