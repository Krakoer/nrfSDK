#include "usb.h"
#include "clock.h"
#include "timer.h"
#include "delay.h"

int main(void)
{
    enable_hfxo();
    unsigned long timer_base = timer_alloc();
    init_delay(timer_base);
    if(!hfxo_enabled()){
        while(1) {}
    }
    usb_init();
    while(1) {
        delay(250);
        usb_write("COUCOU JE SUIS UN COUCOU GRIS\r\n", 31);
    }
}