#include "clock.h"

void enable_hfxo(){
    CLOCK_TASKS_HFCLKSTART = 1;
    while (CLOCK_EVENTS_HFCLKSTARTED == 0) { }
    CLOCK_EVENTS_HFCLKSTARTED = 0;
}

int hfxo_enabled(){
    return CLOCK_HFCLKSTAT & (1<<16 | 1);
}