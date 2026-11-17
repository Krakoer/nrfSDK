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

/// Build an ADV packet to data.
ble_parsing_error_t ble_adv_to_raw(uint8_t *data, unsigned len, ble_adv_packet_t *ble){
    // Buffer length should be at least length of ADV data + 6 bytes of MAC address + 2 bytes of header.
    if(len < ble->adv_len + 8) {
        return BLE_ADV_INVALID_LEN;
    }

    data[0] = ble->header.adv_type;
    data[0] |= ble->header.rx_add << 7;
    data[0] |= ble->header.tx_add << 6;
    // Length is length of adv data + 6 bytes of addr
    data[1] = ble->adv_len + 6;

    for(int i = 0; i < 6; i++){
        data[7-i] = ble->addr[i];
    }

    // Copy data
    for(unsigned int i = 0; i < ble->adv_len; i++){
        data[8+i] = ble->adv_data[i];
    }

    return BLE_ADV_OK;
}

ble_parsing_error_t build_ad_data(ad_item_t *ad_data, unsigned ad_len, uint8_t *out_buf, unsigned out_len, unsigned *bytes_written) {
    unsigned written = 0;
    *bytes_written = 0;
    for(unsigned i = 0; i < ad_len; i++){
        // If we are about to overflow the buffer, return an error
        if(written+ad_data->length+2 > out_len){
            return BLE_ADV_INVALID_LEN;
        }
        out_buf[0] = ad_data->length+1;
        out_buf[1] = ad_data->type;
        for(int j = 0; j < ad_data->length; j++){
            out_buf[2+j] = ad_data->value[j];
        }
        written += ad_data->length+2;
        *bytes_written = written;
        out_buf += ad_data->length+2;
        ad_data++;
    }
    return BLE_ADV_OK;
}