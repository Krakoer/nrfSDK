#pragma once


// UARTE
#define UARTE0_BASE 0x40002000UL

#define UART_TASKS_STARTRX  (*(volatile unsigned long *)(UARTE0_BASE + 0x000))
#define UART_TASKS_STARTTX  (*(volatile unsigned long *)(UARTE0_BASE + 0x008))
#define UART_TASKS_FLUSHRX  (*(volatile unsigned long *)(UARTE0_BASE + 0x02C))
#define UART_EVENTS_RXDRDY  (*(volatile unsigned long *)(UARTE0_BASE + 0x108))
#define UART_EVENTS_ENDRX   (*(volatile unsigned long *)(UARTE0_BASE + 0x110))
#define UART_EVENTS_ENDTX   (*(volatile unsigned long *)(UARTE0_BASE + 0x120))
#define UART_EVENTS_ERROR   (*(volatile unsigned long *)(UARTE0_BASE + 0x124))
#define UART_ENABLE         (*(volatile unsigned long *)(UARTE0_BASE + 0x500))
#define UART_PSEL_TXD       (*(volatile unsigned long *)(UARTE0_BASE + 0x50C))
#define UART_PSEL_RXD       (*(volatile unsigned long *)(UARTE0_BASE + 0x514))
#define UART_BAUDRRATE      (*(volatile unsigned long *)(UARTE0_BASE + 0x524))
#define UART_RDX_PTR        (*(volatile unsigned long *)(UARTE0_BASE + 0x534))
#define UART_RDX_MAXCNT     (*(volatile unsigned long *)(UARTE0_BASE + 0x538))
#define UART_RDX_AMOUNT     (*(volatile unsigned long *)(UARTE0_BASE + 0x53C))
#define UART_TDX_PTR        (*(volatile unsigned long *)(UARTE0_BASE + 0x544))
#define UART_TDX_MAXCNT     (*(volatile unsigned long *)(UARTE0_BASE + 0x548))
#define UART_TDX_AMOUNT     (*(volatile unsigned long *)(UARTE0_BASE + 0x54C))
#define UART_TDX_CONFIG     (*(volatile unsigned long *)(UARTE0_BASE + 0x56C))