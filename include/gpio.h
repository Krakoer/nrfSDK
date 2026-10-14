#pragma once

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