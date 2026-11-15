#pragma once

#define USBD_BASE 0x40027000

#define USBD_EVENTS_USBRESET     (*(volatile unsigned long *)(USBD_BASE + 0x100))
#define USBD_EVENTS_USBEVENT     (*(volatile unsigned long *)(USBD_BASE + 0x158))
#define USBD_EVENTCAUSE          (*(volatile unsigned long *)(USBD_BASE + 0x400))
#define USBD_ENABLE              (*(volatile unsigned long *)(USBD_BASE + 0x500))
#define USBD_USBPULLUP           (*(volatile unsigned long *)(USBD_BASE + 0x504))

#define USBD_EVENTCAUSE_ISOOUTCRC    1
#define USBD_EVENTCAUSE_SUSPEND      1<<8
#define USBD_EVENTCAUSE_RESUME       1<<9
#define USBD_EVENTCAUSE_USBWUALLOWED 1<<10
#define USBD_EVENTCAUSE_READY        1<<11

void usb_init();