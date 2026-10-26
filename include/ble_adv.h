#pragma once

#include <stdint.h>

typedef enum{
	BLE_ADV_OK,
	BLE_ADV_INVALID_LEN
} ble_parsing_error_t;

typedef enum{
	ADV_IND = 0,
	ADV_DIRECT_IND = 1,
	ADV_NONCONN_IND = 2,
	SCAN_REQ = 3,
	SCAN_RSP = 4,
	CONNECT_IND = 5,
	ADV_SCAN_IND = 6,
	ADV_EXT_IND = 7,
	AUX_CONNECT_RSP = 8,
	ADV_UNKNOWN = 0xFF
} ble_adv_type_t;

typedef struct{
    ble_adv_type_t adv_type;
    uint8_t tx_add;
    uint8_t rx_add;
} ble_adv_header_t;

typedef struct{
    ble_adv_header_t header;
	uint8_t addr[6];
	uint8_t *adv_data;
	unsigned adv_len;
} ble_adv_packet_t;

ble_parsing_error_t parse_adv_ble_packet(const uint8_t *data, unsigned len, ble_adv_packet_t *ble);
