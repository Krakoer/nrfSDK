#include "timer.h"

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

int timer_check_event_cmp(unsigned long base, unsigned int cc_reg_num){
    return TIMER_REG(base, TIMER_EVENTS_COMPARE_OFFSET(cc_reg_num));
}

void timer_clear_event_cmp(unsigned long base, unsigned int cc_reg_num){
    TIMER_REG(base, TIMER_EVENTS_COMPARE_OFFSET(cc_reg_num)) = 0;
}