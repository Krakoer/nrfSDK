#pragma once

#include <stdint.h>

#define USBD_BASE 0x40027000

#define USBD_TASKS_STARTEPIN_N(n)   (*(volatile unsigned long *)(USBD_BASE + 0x004 + 0x4UL*(n)))
#define USBD_TASKS_STARTISOIN       (*(volatile unsigned long *)(USBD_BASE + 0x024))
#define USBD_TASKS_STARTEPOUT_N(n)  (*(volatile unsigned long *)(USBD_BASE + 0x028 + 4UL*(n)))
#define USBD_TASKS_EP0RCVOUT        (*(volatile unsigned long *)(USBD_BASE + 0x04C))
#define USBD_TASKS_EP0STATUS        (*(volatile unsigned long *)(USBD_BASE + 0x050))
#define USBD_TASKS_EP0STALL         (*(volatile unsigned long *)(USBD_BASE + 0x054))
#define USBD_EVENTS_USBRESET        (*(volatile unsigned long *)(USBD_BASE + 0x100))
#define USBD_EVENTS_ENDEPIN_N(n)    (*(volatile unsigned long *)(USBD_BASE + 0x108 + 4UL*(n)))
#define USBD_EVENTS_EP0DATADONE     (*(volatile unsigned long *)(USBD_BASE + 0x128))
#define USBD_EVENTS_ENDEPOUT_N(n)   (*(volatile unsigned long *)(USBD_BASE + 0x130 + 4UL*(n)))
#define USBD_EVENTS_EP0SETUP        (*(volatile unsigned long *)(USBD_BASE + 0x15C))
#define USBD_EVENTS_USBEVENT        (*(volatile unsigned long *)(USBD_BASE + 0x158))
#define USBD_EVENTS_EPDATA          (*(volatile unsigned long *)(USBD_BASE + 0x160))
#define USBD_SHORTS                 (*(volatile unsigned long *)(USBD_BASE + 0x200))
#define USBD_INTENSET               (*(volatile unsigned long *)(USBD_BASE + 0x304))
#define USBD_EVENTCAUSE             (*(volatile unsigned long *)(USBD_BASE + 0x400))
#define USBD_EPDATASTATUS           (*(volatile unsigned long *)(USBD_BASE + 0x46C))
#define USBD_USBADDR                (*(volatile unsigned long *)(USBD_BASE + 0x470))
#define USBD_BMREQUESTTYPE          (*(volatile unsigned long *)(USBD_BASE + 0x480)) // SETUP data, byte 0, bmRequestType
#define USBD_BREQUEST               (*(volatile unsigned long *)(USBD_BASE + 0x484)) // SETUP data, byte 1, bRequest
#define USBD_WVALUEL                (*(volatile unsigned long *)(USBD_BASE + 0x488)) // SETUP data, byte 2, LSB of wValue
#define USBD_WVALUEH                (*(volatile unsigned long *)(USBD_BASE + 0x48C)) // SETUP data, byte 3, MSB of wValue
#define USBD_WINDEXL                (*(volatile unsigned long *)(USBD_BASE + 0x490)) // SETUP data, byte 4, LSB of wIndex
#define USBD_WINDEXH                (*(volatile unsigned long *)(USBD_BASE + 0x494)) // SETUP data, byte 5, MSB of wIndex
#define USBD_WLENGTHL               (*(volatile unsigned long *)(USBD_BASE + 0x498)) // SETUP data, byte 6, LSB of wLength
#define USBD_WLENGTHH               (*(volatile unsigned long *)(USBD_BASE + 0x49C)) // SETUP data, byte 7, MSB of wLength
#define USBD_SIZE_EPOUT_N(n)        (*(volatile unsigned long *)(USBD_BASE + 0x4A0 + 4UL*(n)))
#define USBD_ENABLE                 (*(volatile unsigned long *)(USBD_BASE + 0x500))
#define USBD_USBPULLUP              (*(volatile unsigned long *)(USBD_BASE + 0x504))
#define USBD_EPINEN                 (*(volatile unsigned long *)(USBD_BASE + 0x510))
#define USBD_EPOUTEN                (*(volatile unsigned long *)(USBD_BASE + 0x514))
#define USBD_EPIN_PTR_N(n)          (*(volatile unsigned long *)(USBD_BASE + 0x600 + 0x14UL*(n)))
#define USBD_EPIN_MAXCNT_N(n)       (*(volatile unsigned long *)(USBD_BASE + 0x604 + 0x14UL*(n)))
#define USBD_EPIN_AMOUNT_N(n)       (*(volatile unsigned long *)(USBD_BASE + 0x608 + 0x14UL*(n)))
#define USBD_EPOUT_PTR_N(n)         (*(volatile unsigned long *)(USBD_BASE + 0x700 + 0x14UL*(n)))
#define USBD_EPOUT_MAXCNT_N(n)      (*(volatile unsigned long *)(USBD_BASE + 0x704 + 0x14UL*(n)))
#define USBD_EPOUT_AMOUNT_N(n)      (*(volatile unsigned long *)(USBD_BASE + 0x708 + 0x14UL*(n)))

#define USBD_INT_USBRESET_MSK 1<<0
#define USBD_INT_STARTED_MSK 1<<1
#define USBD_INT_ENDEPIN_0_MSK 1<<2
#define USBD_INT_ENDEPIN_1_MSK 1<<3
#define USBD_INT_ENDEPIN_2_MSK 1<<4
#define USBD_INT_ENDEPIN_3_MSK 1<<5
#define USBD_INT_ENDEPIN_4_MSK 1<<6
#define USBD_INT_ENDEPIN_5_MSK 1<<7
#define USBD_INT_ENDEPIN_6_MSK 1<<8
#define USBD_INT_ENDEPIN_7_MSK 1<<9
#define USBD_INT_EP0DATADONE_MSK 1<<10
#define USBD_INT_ENDISOIN_MSK 1<<11
#define USBD_INT_ENDEPOUT_0_MSK 1<<12
#define USBD_INT_ENDEPOUT_1_MSK 1<<13
#define USBD_INT_ENDEPOUT_2_MSK 1<<14
#define USBD_INT_ENDEPOUT_3_MSK 1<<15
#define USBD_INT_ENDEPOUT_4_MSK 1<<16
#define USBD_INT_ENDEPOUT_5_MSK 1<<17
#define USBD_INT_ENDEPOUT_6_MSK 1<<18
#define USBD_INT_ENDEPOUT_7_MSK 1<<19
#define USBD_INT_ENDISOOUT_MSK 1<<20
#define USBD_INT_SOF_MSK 1<<21
#define USBD_INT_USBEVENT_MSK 1<<22
#define USBD_INT_EP0SETUP_MSK 1<<23
#define USBD_INT_EPDATA_MSK 1<<24

#define USBD_STD_GET_STATUS 0
#define USBD_STD_CLEAR_FEATURE 1
#define USBD_STD_SET_FEATURE 3
#define USBD_STD_SET_ADDRESS 5
#define USBD_STD_GET_DESCRIPTOR 6
#define USBD_STD_SET_DESCRIPTOR 7
#define USBD_STD_GET_CONFIGURATION 8
#define USBD_STD_SET_CONFIGURATION 9
#define USBD_STD_GET_INTERFACE 10
#define USBD_STD_SET_INTERFACE 11
#define USBD_STD_SYNCH_FRAME 12

#define USBD_EVENTCAUSE_ISOOUTCRC    1
#define USBD_EVENTCAUSE_SUSPEND      1<<8
#define USBD_EVENTCAUSE_RESUME       1<<9
#define USBD_EVENTCAUSE_USBWUALLOWED 1<<10
#define USBD_EVENTCAUSE_READY        1<<11

typedef enum{
    CONTROL_IDLE,
    CONTROL_DATA_IN,
    CONTROL_DATA_OUT,
} usb_control_state_t;

typedef struct{
    usb_control_state_t state;
    uint8_t *ptr;
    uint32_t remaining;
    int ep0_busy;
    int out_ready;
    int out_armed;
} usb_control_machine_t;


void usb_init();
void usb_write(const uint8_t *buf, int len);
void usb_poll();