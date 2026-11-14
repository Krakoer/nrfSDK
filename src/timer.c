#include "timer.h"

static uint32_t taken_timers = 0;

static const unsigned long TIMERS[] = {
    TIMER0_BASE,
    TIMER1_BASE,
    TIMER2_BASE,
    TIMER3_BASE,
    TIMER4_BASE,
};

void timer_start(unsigned long base){
    TIMER_REG(base, TIMER_TASKS_START_OFFSET) = 1;
}

void timer_stop(unsigned long base){
    TIMER_REG(base, TIMER_TASKS_STOP_OFFSET) = 1;
}

void timer_clear(unsigned long base){
    TIMER_REG(base, TIMER_TASKS_CLEAR_OFFSET) = 1;
}

void timer_set_mode(unsigned long base, unsigned int mode){
    TIMER_REG(base, TIMER_MODE_OFFSET) = mode;
}

void timer_set_bitmode(unsigned long base, unsigned int bitmode){
    TIMER_REG(base, TIMER_BITMODE_OFFSET) = bitmode;
}

void timer_set_prescaler(unsigned long base, uint8_t prescaler){
    TIMER_REG(base, TIMER_PRESCALER_OFFSET) = prescaler & 0x0F;
}

void timer_set_cc_value(unsigned long base, int cc_reg_num, unsigned long value){
    if(base == TIMER0_BASE || base == TIMER1_BASE || base == TIMER2_BASE){
        if(cc_reg_num < 0 || cc_reg_num > 3){
            return;
        }
    }
    else if(base == TIMER3_BASE || base == TIMER4_BASE){
        if(cc_reg_num < 0 || cc_reg_num > 5){
            return;
        }
    }

    TIMER_REG(base, TIMER_CC_OFFSET(cc_reg_num)) = value;
}

void timer_set_cmp_clear_short(unsigned long base, unsigned int cc_reg_num){
    TIMER_REG(base, TIMER_SHORTS_OFFSET) |= (1UL << cc_reg_num);
}

void timer_set_cmp_stop_short(unsigned long base, unsigned int cc_reg_num){
    TIMER_REG(base, TIMER_SHORTS_OFFSET) |= (1UL << (cc_reg_num+8));
}

int timer_check_event_cmp(unsigned long base, unsigned int cc_reg_num){
    return TIMER_REG(base, TIMER_EVENTS_COMPARE_OFFSET(cc_reg_num));
}

void timer_clear_event_cmp(unsigned long base, unsigned int cc_reg_num){
    TIMER_REG(base, TIMER_EVENTS_COMPARE_OFFSET(cc_reg_num)) = 0;
}

void timer_capture(unsigned long base, unsigned int cc_reg_num){
    TIMER_REG(base, TIMER_TASKS_CAPTURE_OFFSET(cc_reg_num)) = 1;
}

unsigned long timer_alloc(){
        // Allocate the first free channel
    int i = 0;
    while(i < 5 && ((taken_timers & (1<<i)) != 0)){
        i++;
    }
    // If all timers are taken, return -1
    if(i == 5){
        return -1;
    }
    // Alloc and return
    taken_timers |= (1<<i);
    return TIMERS[i];
}

void timer_free(unsigned long timer){
    for(int i = 0; i < 5; i++){
        if(timer == TIMERS[i]){
            taken_timers &= ~(1 << timer);
            return;
        }
    }
}