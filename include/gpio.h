#pragma once

#define GPIO_P0_BASE 0x50000000UL
#define GPIO_P1_BASE 0x50000300UL

#define GPIO_OUT_OFFSET     0x504
#define GPIO_OUTSET_OFFSET  0x508
#define GPIO_OUTCLR_OFFSET  0x50C
#define GPIO_IN_OFFSET      0x510
#define GPIO_DIR_OFFSET     0x514
#define GPIO_DIRSET_OFFSET  0x518
#define GPIO_DIRCLR_OFFSET  0x51C
#define GPIO_PIN_CNF_OFFSET(n) (0x700UL + 4UL*(n))

#define GPIO_REG(base, offset) \
    (*(volatile unsigned long *)((base) + (offset)))

typedef struct {
    unsigned long base;
    unsigned      pin;
} gpio_pin_t;

void gpio_set_output(gpio_pin_t p);
void gpio_set_input(gpio_pin_t p);
void gpio_enable_pullup(gpio_pin_t p);
void gpio_output_high(gpio_pin_t p);
void gpio_output_low(gpio_pin_t p);
int gpio_read(gpio_pin_t p);