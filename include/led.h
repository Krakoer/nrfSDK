#pragma once

#include "gpio.h"

#define LED1       ((gpio_pin_t){GPIO_P0_BASE, 6})
#define LED2_RED   ((gpio_pin_t){GPIO_P0_BASE, 8})
#define LED2_GREEN ((gpio_pin_t){GPIO_P1_BASE, 9})
#define LED2_BLUE  ((gpio_pin_t){GPIO_P0_BASE, 12})

void led_init(gpio_pin_t led);
void led_on(gpio_pin_t led);
void led_off(gpio_pin_t led);