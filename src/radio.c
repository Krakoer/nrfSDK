#include "radio.h"
#include <stdint.h>

#define RADIO_BUF_SIZE 257
static volatile uint8_t radio_buf[RADIO_BUF_SIZE] = {0};

static inline unsigned long radio_pcnf0_encode(radio_pcnf0_t cfg)
{
    return  ((cfg.lflen & 0xFUL)        << CNF0_LFLEN_POS)
          | ((cfg.s0len & 0x1UL)        << CNF0_S0LEN_POS)
          | ((cfg.s1len & 0xFUL)        << CNF0_S1LEN_POS)
          | (((unsigned long)cfg.plen)  << CNF0_PLEN_POS);
}

static inline unsigned long radio_pcnf1_encode(radio_pcnf1_t cfg)
{
    return  ((cfg.maxlen & 0xFFUL)      << CNF1_MAXLEN_POS)
          | ((cfg.statlen & 0xFFUL)     << CNF1_STATLEN_POS)
          | ((cfg.balen & 0x7UL)        << CNF1_BALEN_POS)
          | ((cfg.endian & 0x1UL)       << CNF1_ENDIAN_POS)
          | ((cfg.whiteen & 0x1UL)      << CNF1_WHITEEN_POS);
}

void radio_configure(const radio_config_t *cfg){
    RADIO_MODE = cfg->mode;
    RADIO_PCNF0 = radio_pcnf0_encode(cfg->pcnf0);
    RADIO_PCNF1 = radio_pcnf1_encode(cfg->pcnf1);
    RADIO_BASE0 = cfg->base0;
    RADIO_PREFIX0 = cfg->prefix0;
    RADIO_DATAWHITEIV = cfg->datawhiteiv;
    RADIO_RXADDRESSES = cfg->rxaddr;
    RADIO_FREQUENCY = cfg->frequency;
    RADIO_CRCCNF = cfg->crccnf;
    RADIO_CRCPOLY = cfg->crcpoly;
    RADIO_CRCINIT = cfg->crcinit;
}

void wait_for_radio_ready(){
    while(RADIO_EVENTS_READY == 0) { }
    RADIO_EVENTS_READY = 0;
}

void radio_start_rx(){
    RADIO_PACKET_PTR = (long unsigned int)&radio_buf;
    RADIO_EVENTS_READY = 0;
    RADIO_TASKS_RXEN = 1;
    while(RADIO_EVENTS_READY == 0) { }
    RADIO_EVENTS_READY = 0;
}

void radio_rx(radio_packet_t *pckt){
    RADIO_TASKS_START = 1;
    while (RADIO_EVENTS_END == 0) { }
    RADIO_EVENTS_END = 0;

    pckt->len = radio_buf[1] + 2; // 2 because S0 + length
    pckt->crc_status = RADIO_CRCSTATUS;
    unsigned int l = pckt->len;

    if(l > pckt->buf_len) l = pckt->buf_len;

    for(unsigned int i = 0; i < l; i++){
        pckt->payload[i] = radio_buf[i];
    }
}

void radio_disable(){
    RADIO_EVENTS_DISABLE = 0;
    RADIO_TASKS_DISABLE = 1;
    while(RADIO_EVENTS_DISABLE == 0) { }
    RADIO_EVENTS_DISABLE = 0;
}