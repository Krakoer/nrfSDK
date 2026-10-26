#pragma once

#include <stdint.h>

#define RADIO_BASE 0x40001000
#define RADIO_TASKS_TXEN (*(volatile unsigned long *)(RADIO_BASE + 0x000))
#define RADIO_TASKS_RXEN (*(volatile unsigned long *)(RADIO_BASE + 0x004))
#define RADIO_TASKS_START (*(volatile unsigned long *)(RADIO_BASE + 0x008))
#define RADIO_TASKS_STOP (*(volatile unsigned long *)(RADIO_BASE + 0x00C))
#define RADIO_TASKS_DISABLE (*(volatile unsigned long *)(RADIO_BASE + 0x010))
#define RADIO_EVENTS_READY (*(volatile unsigned long *)(RADIO_BASE + 0x100))
#define RADIO_EVENTS_END (*(volatile unsigned long *)(RADIO_BASE + 0x10C))
#define RADIO_EVENTS_DISABLE (*(volatile unsigned long *)(RADIO_BASE + 0x110))
#define RADIO_CRCSTATUS (*(volatile unsigned long *)(RADIO_BASE + 0x400))
#define RADIO_PACKET_PTR (*(volatile unsigned long *)(RADIO_BASE + 0x504))
#define RADIO_FREQUENCY (*(volatile unsigned long *)(RADIO_BASE + 0x508))
#define RADIO_MODE (*(volatile unsigned long *)(RADIO_BASE + 0x510))
// Disabled  0  RADIO is in the Disabled state
// RxRu      1  RADIO is in the RXRU state
// RxIdle    2  RADIO is in the RXIDLE state
// Rx        3  RADIO is in the RX state
// RxDisable 4  RADIO is in the RXDISABLED state
// TxRu      9  RADIO is in the TXRU state
// TxIdle    10 RADIO is in the TXIDLE state
// Tx        11 RADIO is in the TX state
// TxDisable 12 RADIO is in the TXDISABLED state
#define RADIO_PCNF0 (*(volatile unsigned long *)(RADIO_BASE + 0x514))
#define RADIO_PCNF1 (*(volatile unsigned long *)(RADIO_BASE + 0x518))
#define RADIO_BASE0 (*(volatile unsigned long *)(RADIO_BASE + 0x51C))
#define RADIO_PREFIX0 (*(volatile unsigned long *)(RADIO_BASE + 0x524))
#define RADIO_RXADDRESSES (*(volatile unsigned long *)(RADIO_BASE + 0x530))
#define RADIO_CRCCNF (*(volatile unsigned long *)(RADIO_BASE + 0x534))
#define RADIO_CRCPOLY (*(volatile unsigned long *)(RADIO_BASE + 0x538))
#define RADIO_CRCINIT (*(volatile unsigned long *)(RADIO_BASE + 0x53C))
#define RADIO_STATE (*(volatile unsigned long *)(RADIO_BASE + 0x550))
#define RADIO_DATAWHITEIV (*(volatile unsigned long *)(RADIO_BASE + 0x554))

#define CNF0_LFLEN_POS 0UL
#define CNF0_S0LEN_POS 8UL
#define CNF0_S1LEN_POS 16UL
#define CNF0_CILEN_POS 22UL
#define CNF0_PLEN_POS 24UL
#define CNF0_CRCINC_POS 26UL
#define CNF0_TERMLEN_POS 28UL

#define CNF1_MAXLEN_POS 0UL
#define CNF1_STATLEN_POS 8UL
#define CNF1_BALEN_POS 16UL
#define CNF1_ENDIAN_POS 24UL
#define CNF1_WHITEEN_POS 25UL

typedef enum {
    PLEN_8BITS = 0,
    PLEN_16BITS = 1,
    PLEN_32BITS = 2,
    PLEN_LONGRANGE = 3,
} radio_plen_t;

typedef enum {
    MODE_NRF_1MBIT = 0, // 1 Mbit/s Nordic proprietary radio mode
    MODE_NRF_2MBIT = 1, // 2 Mbit/s Nordic proprietary radio mode
    MODE_BLE_1MBIT = 3, // 1 Mbit/s BLE
    MODE_BLE_2MBIT = 4, // 2 Mbit/s BLE
    MODE_BLE_LR125KBIT = 5, // Long range 125 kbit/s TX, 125 kbit/s and 500 kbit/s RX
    MODE_BLE_LR500KBIT = 6, // Long range 500 kbit/s TX, 125 kbit/s and 500 kbit/s RX
} radio_mode_t;

typedef struct {
    unsigned     lflen;   // Length on air of LENGTH field in number of bits.
    unsigned     s0len;   // Length on air of S0 field in number of bytes.
    unsigned     s1len;   // Length on air of S1 field in number of bits.
    radio_plen_t plen;    // Length of preamble on air. Decision point: TASKS_START task.
} radio_pcnf0_t;

typedef struct {
    unsigned     maxlen;  // Maximum length of packet payload.
    unsigned     statlen; // Static length in number of bytes.
    unsigned     balen;   // Base address length in number of bytes.
    unsigned     endian;   // On air endianness of packet, this applies to the S0, LENGTH, S1 and the PAYLOAD fields.
    unsigned     whiteen;   // Enable or disable packet whitening.
} radio_pcnf1_t;

typedef struct {
    radio_mode_t mode;
    radio_pcnf0_t pcnf0;
    radio_pcnf1_t pcnf1;
    unsigned base0;
    unsigned prefix0;
    unsigned datawhiteiv;
    unsigned rxaddr;
    unsigned frequency;
    unsigned crccnf;
    unsigned crcinit;
    unsigned crcpoly;
} radio_config_t;

typedef struct {
    uint8_t *payload;
    unsigned len;
    unsigned buf_len;
    int crc_status;
} radio_packet_t;

void radio_configure(const radio_config_t *cfg);
void wait_for_radio_ready();
void radio_start_rx();
void radio_disable();
void radio_rx(radio_packet_t *pckt);