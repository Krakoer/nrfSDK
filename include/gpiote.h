#pragma once

#include "gpio.h"

#define GPIOTE_BASE 0x40006000UL

#define GPIOTE_TASKS_OUT_N(n) (*(volatile unsigned long *)(GPIOTE_BASE + 0x000UL + 4UL*(n)))
#define GPIOTE_TASKS_SET_N(n) (*(volatile unsigned long *)(GPIOTE_BASE + 0x030UL + 4UL*(n)))
#define GPIOTE_TASKS_CLR_N(n) (*(volatile unsigned long *)(GPIOTE_BASE + 0x060UL + 4UL*(n)))
#define GPIOTE_EVENTS_IN_N(n) (*(volatile unsigned long *)(GPIOTE_BASE + 0x100UL + 4UL*(n)))
#define GPIOTE_EVENTS_PORT    (*(volatile unsigned long *)(GPIOTE_BASE + 0x17C))
#define GPIOTE_INTENSET       (*(volatile unsigned long *)(GPIOTE_BASE + 0x304))
#define GPIOTE_INTENCLR       (*(volatile unsigned long *)(GPIOTE_BASE + 0x308))
#define GPIOTE_CONFIG_N(n)    (*(volatile unsigned long *)(GPIOTE_BASE + 0x510 + 4UL*(n)))

typedef enum {
    GPIOTE_MODE_DISABLED = 0,
    GPIOTE_MODE_EVENT = 1,
    GPIOTE_MODE_TASK = 3,
} gpiote_mode_t;

typedef enum {
    GPIOTE_POLARITY_NONE = 0,
    GPIOTE_POLARITY_LOWTOHIGH = 1,
    GPIOTE_POLARITY_HIGHTOLOW = 2,
    GPIOTE_POLARITY_TOGGLE = 3,
} gpiote_polarity_t;

typedef struct {
    gpiote_mode_t     mode;
    gpio_pin_t        pin;
    gpiote_polarity_t polarity;
    unsigned          outinit;
} gpiote_config_t;

void gpiote_configure(int n, gpiote_config_t *config);