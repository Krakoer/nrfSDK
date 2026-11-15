#include "usb.h"
#include "power.h"
#include <stdint.h>

void usb_init(){
    while(POWER_EVENTS_USBDETECTED == 0){ }
    POWER_EVENTS_USBDETECTED = 0;

    // https://docs.nordicsemi.com/r/bundle/errata_nrf52840_engc/page/err/nrf52840/engineeringc/latest/anomaly_840_171.html
    if(*(volatile uint32_t *)0x4006EC00 == 0x00000000) {
        *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    }
    *(volatile uint32_t *)0x4006EC14 = 0x000000C0; 
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;

    USBD_ENABLE = 1;

    while(1) {
        if(USBD_EVENTS_USBEVENT != 0){
            USBD_EVENTS_USBEVENT = 0;
            if((USBD_EVENTCAUSE & USBD_EVENTCAUSE_READY) != 0){
                break;
            }
        }
    }

    // https://docs.nordicsemi.com/r/bundle/errata_nrf52840_engc/page/err/nrf52840/engineeringc/latest/anomaly_840_171.html
    if(*(volatile uint32_t *)0x4006EC00 == 0x00000000) {
        *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    }
    *(volatile uint32_t *)0x4006EC14 = 0x00000000; 
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;

    while(POWER_EVENTS_USBPWRRDY == 0) { }
    POWER_EVENTS_USBPWRRDY = 0;

    USBD_USBPULLUP = 1;
}