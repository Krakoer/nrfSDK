#include "timer.h"
#include "delay.h"

static unsigned long timer_base = 0;

void init_delay(unsigned long tb){
    timer_base = tb;
    timer_set_prescaler(timer_base, 4);
    timer_set_mode(timer_base, TIMER_MODE_TIMER);
    timer_set_bitmode(timer_base, TIMER_BITMODE_32BIT);
    timer_set_cmp_clear_short(timer_base, 0);
    timer_set_cmp_stop_short(timer_base, 0);
}

void delay(int millis){
    if(timer_base != 0){
        timer_set_cc_value(timer_base, 0, 1000*millis);
        timer_start(timer_base);
        while(timer_check_event_cmp(timer_base, 0) == 0){ }
        timer_clear_event_cmp(timer_base, 0);
    }
}