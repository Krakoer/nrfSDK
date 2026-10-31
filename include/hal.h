#pragma once

#include <stdint.h>

static inline void nrf_event_clear(volatile unsigned long *evt){
    *evt = 0;
    // Cortex-M4 buffers the writes, so read the value will force the core to wait for the write to actually happen (see eg. nrf_radio_event_clear in NRF SDK)
    uint32_t dum = *evt;
    (void)dum;
}

static inline void __set_PRIMASK(uint32_t priMask)
{
  __asm__ volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

#define __WFI()                             __asm__ volatile ("wfi")