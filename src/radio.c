#include "radio.h"
#include <stdint.h>

#define RADIO_BUF_SIZE 257
static volatile uint8_t radio_buf1[RADIO_BUF_SIZE] = {0};
static volatile uint8_t radio_buf2[RADIO_BUF_SIZE] = {0};
static uint8_t buf_num = 1;

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
    if(buf_num == 2){
        RADIO_PACKET_PTR = (long unsigned int)&radio_buf1;
        buf_num = 1;
    } else{
        RADIO_PACKET_PTR = (long unsigned int)&radio_buf2;
        buf_num = 2;
    }
    RADIO_EVENTS_READY = 0;
    RADIO_TASKS_RXEN = 1;
    while(RADIO_EVENTS_READY == 0) { }
    RADIO_EVENTS_READY = 0;
}

void radio_rx_start(){
    RADIO_TASKS_START = 1;
}

int radio_rx_poll(){
    if(RADIO_EVENTS_ADDRESS) {
        RADIO_EVENTS_ADDRESS = 0;
        if(buf_num == 1){
            RADIO_PACKET_PTR = (long unsigned int)&radio_buf2;
            buf_num = 2;
        } else{
            RADIO_PACKET_PTR = (long unsigned int)&radio_buf1;
            buf_num = 1;
        }
    }
    return RADIO_EVENTS_END;
}

void radio_rx_fetch(radio_packet_t *pckt){
    RADIO_EVENTS_END = 0;
    if(buf_num == 2){
        pckt->len = radio_buf1[1] + 2; // 2 because S0 + length
    } else{
        pckt->len = radio_buf2[1] + 2; // 2 because S0 + length
    }
    pckt->crc_status = RADIO_CRCSTATUS;
    unsigned int l = pckt->len;

    if(l > pckt->buf_len) l = pckt->buf_len;

    if(buf_num == 1) {
        for(unsigned int i = 0; i < l; i++){
            pckt->payload[i] = radio_buf2[i];
        }
    } else {
        for(unsigned int i = 0; i < l; i++){
            pckt->payload[i] = radio_buf1[i];
        }    
    }
}

void radio_disable(){
    RADIO_EVENTS_DISABLE = 0;
    RADIO_TASKS_DISABLE = 1;
    while(RADIO_EVENTS_DISABLE == 0) { }
    RADIO_EVENTS_DISABLE = 0;
}

void radio_enable_short(int short_num){
    RADIO_SHORTS |= 1UL<<short_num;
}

void radio_disable_short(int short_num){
    RADIO_SHORTS &= ~(1UL<<short_num);
}