#include "gpio.h"
#include "led.h"
#include "nrf52_regs.h"
#include "serial.h"

int main(void)
{
    const gpio_pin_t tx = {
        GPIO_P0_BASE,
        17
    };
    const gpio_pin_t rx = {
        GPIO_P0_BASE,
        20
    };

    serial_init(tx, rx, BAUD115200);
    led_init(LED1);

    gpio_pin_t btn_pin = {
        GPIO_P1_BASE,
        6
    };
    gpio_enable_pullup(btn_pin);

    int cur_state = gpio_read(btn_pin);

    if(cur_state){ // High is button up
        led_on(LED1);
    } else{
        led_off(LED1);
    }
    while (1) {
        int new_state = gpio_read(btn_pin);
        // Button changed state
        if(cur_state != new_state) {
            if(new_state){
                led_on(LED1);
            } else{
                led_off(LED1);
            }
            cur_state = new_state;
        }
    }
}