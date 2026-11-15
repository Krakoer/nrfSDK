#pragma once

#define POWER_BASE 0x40000000

#define POWER_EVENTS_USBDETECTED (*(volatile unsigned long *)(POWER_BASE + 0x11C)) // Voltage supply detected on VBUS
#define POWER_EVENTS_USBREMOVED (*(volatile unsigned long *)(POWER_BASE + 0x120)) // Voltage supply removed from VBUS
#define POWER_EVENTS_USBPWRRDY (*(volatile unsigned long *)(POWER_BASE + 0x124)) // USB 3.3 V supply read
#define POWER_USBREGSTATUS (*(volatile unsigned long *)(POWER_BASE + 0x438)) // USB supply status