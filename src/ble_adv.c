#include "ble_ad.h"
#include "ble_adv.h"

ble_parsing_error_t parse_adv_ble_packet(const uint8_t *data, unsigned len, ble_adv_packet_t *ble){
    // Valid ADV packet should be at least 2 (header) + 6 (address) + 3 (1 adv data)
    if(len < 11){
        return BLE_ADV_INVALID_LEN;
    }
    if(data[1]+2UL != len) {
        return BLE_ADV_INVALID_LEN;
    }

    ble_adv_header_t header;

    switch(data[0] & 0x0F){
        case 0:
            header.adv_type = ADV_IND;
            break;
        case 1:
            header.adv_type = ADV_DIRECT_IND;
            break;
        case 2:
            header.adv_type = ADV_NONCONN_IND;
            break;
        case 3:
            header.adv_type = SCAN_REQ;
            break;
        case 4:
            header.adv_type = SCAN_RSP;
            break;
        case 5:
            header.adv_type = CONNECT_IND;
            break;
        case 6:
            header.adv_type = ADV_SCAN_IND;
            break;
        case 7:
            header.adv_type = ADV_EXT_IND;
            break;
        case 8:
            header.adv_type = AUX_CONNECT_RSP;
            break;
        default:
            header.adv_type = ADV_UNKNOWN;
            break;
    }

    header.rx_add = (data[0] & 0x80) != 0;
    header.tx_add = (data[0] & 0x40) != 0;
    ble->header = header;

    for(int i = 0; i < 6; i++){
        ble->addr[i] = data[7-i];
    }

    ble->adv_data = &data[8];
    ble->adv_len = len-8;

    return BLE_ADV_OK;
}