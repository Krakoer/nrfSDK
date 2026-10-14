#include "led.h"

void led_init(gpio_pin_t led){
    gpio_set_output(led);
}

void led_on(gpio_pin_t led){
    gpio_output_high(led);
}

void led_off(gpio_pin_t led){
    gpio_output_low(led);
}