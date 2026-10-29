#include "radio.h"
#include "nvic.h"
#include "hal.h"
#include <stdint.h>

#define RADIO_BUF_SIZE 257
static volatile uint8_t radio_buf1[RADIO_BUF_SIZE] = {0};
static volatile uint8_t radio_buf2[RADIO_BUF_SIZE] = {0};
static unsigned int bufnum = 1;

static radio_packet_t* radio_slot;
static unsigned slot_status = 0;
static unsigned dropped = 0;
static unsigned received = 0;

static void (*volatile irq_handler)(void);

void RADIO_IRQHandler(void){
    if(RADIO_EVENTS_ADDRESS){
        nrf_event_clear(&RADIO_EVENTS_ADDRESS);
        if(bufnum == 1){
            bufnum = 2;
            RADIO_PACKET_PTR = (long unsigned int)radio_buf2;
        } else{
            bufnum = 1;
            RADIO_PACKET_PTR = (long unsigned int)radio_buf1;
        }
    }
    if(RADIO_EVENTS_END){
        nrf_event_clear(&RADIO_EVENTS_END);
        received ++;
        // Check that slot has been read
        if(slot_status == 0){
            if(bufnum == 1){
                radio_slot->len = radio_buf2[1] + 2; // 2 because S0 + length
            }
            else{
                radio_slot->len = radio_buf1[1] + 2; // 2 because S0 + length
            }
            radio_slot->crc_status = RADIO_CRCSTATUS;
            unsigned int l = radio_slot->len;
            if(l > radio_slot->buf_len) l = radio_slot->buf_len;
            if(bufnum == 1){
                for(unsigned int i = 0; i < l; i++){
                    radio_slot->payload[i] = radio_buf2[i];
                }
            }else{
                for(unsigned int i = 0; i < l; i++){
                    radio_slot->payload[i] = radio_buf1[i];
                } 
            }
            slot_status = 1;
        }
        else{
            dropped++;
        }
        // if(irq_handler){
        //     irq_handler();
        // }
    }
}

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

void radio_enable_rx(){
    RADIO_PACKET_PTR = (long unsigned int)&radio_buf1;

    RADIO_EVENTS_READY = 0;
    RADIO_TASKS_RXEN = 1;
    while(RADIO_EVENTS_READY == 0) { }
    RADIO_EVENTS_READY = 0;
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

void radio_enable_irq(int int_n){
    RADIO_INTENSET = (1UL<<int_n);
    NVIC_ISER0 = (1UL<<1); // RADIO ID is 1
}

void radio_register_irq_handler(radio_irq_cb_t handler){
    irq_handler = handler;
}

void radio_set_rx_slot(radio_packet_t *slot){
    radio_slot = slot;
}

int radio_packet_ready(){
    return slot_status == 1;
}

void radio_slot_available(){
    slot_status = 0;
}

unsigned radio_get_dropped_count(){
    return dropped;
}

unsigned radio_get_received_count(){
    return received;
}