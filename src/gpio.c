#include "gpio.h"
#include "nrf52_regs.h"

void gpio_set_output(gpio_pin_t p){
    GPIO_REG(p.base, GPIO_DIRSET_OFFSET) = 1UL << p.pin;
}

void gpio_set_input(gpio_pin_t p){
    GPIO_REG(p.base, GPIO_DIRCLR_OFFSET) = 1UL << p.pin;
}

void gpio_enable_pullup(gpio_pin_t p){
    GPIO_REG(p.base, GPIO_PIN_CNF_OFFSET(p.pin)) = 3UL << 2;
}

void gpio_output_high(gpio_pin_t p){
    GPIO_REG(p.base, GPIO_OUTCLR_OFFSET) = 1UL << p.pin;
}

void gpio_output_low(gpio_pin_t p){
    GPIO_REG(p.base, GPIO_OUTSET_OFFSET) = 1UL << p.pin;
}

int gpio_read(gpio_pin_t p){
    return ((GPIO_REG(p.base, GPIO_IN_OFFSET) & (1UL << p.pin)) == 0);
}