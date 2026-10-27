#pragma once

#include <stdint.h>

#define TIMER0_BASE 0x40008000UL
#define TIMER1_BASE 0x40009000UL
#define TIMER2_BASE 0x4000A000UL
#define TIMER3_BASE 0x4001A000UL
#define TIMER4_BASE 0x4001B000UL

#define TIMER_TASKS_START_OFFSET       0x000
#define TIMER_TASKS_STOP_OFFSET        0x004
#define TIMER_TASKS_COUNT_OFFSET       0x008
#define TIMER_TASKS_CLEAR_OFFSET       0x00C
#define TIMER_TASKS_CAPTURE_OFFSET(n)  (0x040UL + 4UL*(n))
#define TIMER_EVENTS_COMPARE_OFFSET(n) (0x140UL + 4UL*(n))
#define TIMER_SHORTS_OFFSET            0x200
#define TIMER_INTENSET_OFFSET          0x304
#define TIMER_INTENCLR_OFFSET          0x308
#define TIMER_MODE_OFFSET              0x504
#define TIMER_BITMODE_OFFSET           0x508
#define TIMER_PRESCALER_OFFSET         0x510
#define TIMER_CC_OFFSET(n)            (0x540UL + 4UL*(n))

#define TIMER_REG(base, offset) \
    (*(volatile unsigned long *)((base) + (offset)))

#define TIMER_MODE_TIMER   0
#define TIMER_MODE_COUNTER 1

#define TIMER_BITMODE_16BIT  0
#define TIMER_BITMODE_8BIT   1
#define TIMER_BITMODE_24BIT  2
#define TIMER_BITMODE_32BIT  3

void timer_start(unsigned long base);
void timer_stop(unsigned long base);
void timer_clear(unsigned long base);
void timer_set_mode(unsigned long base, unsigned int mode);
void timer_set_bitmode(unsigned long base, unsigned int bitmode);
void timer_set_prescaler(unsigned long base, uint8_t prescaler);
void timer_set_cc_value(unsigned long base, int cc_reg_num, unsigned long value);
void timer_set_cmp_clear_short(unsigned long base, unsigned int cc_reg_num);
int timer_check_event_cmp(unsigned long base, unsigned int cc_reg_num);
void timer_clear_event_cmp(unsigned long base, unsigned int cc_reg_num);