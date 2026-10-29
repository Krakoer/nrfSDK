#pragma once

#include <stdint.h>

static inline void nrf_event_clear(volatile unsigned long *evt){
    *evt = 0;
    // Cortex-M4 buffers the writes, so read the value will force the core to wait for the write to actually happen (see eg. nrf_radio_event_clear in NRF SDK)
    uint32_t dum = *evt;
    (void)dum;
}