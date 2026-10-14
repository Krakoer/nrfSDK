#include "serial.h"
#include "nrf52_regs.h"

#define SERIAL_BUF_SIZE 256

static char serial_buf[SERIAL_BUF_SIZE] = {0};

int strlen(const char *str){
    const char *p;
    if (str == 0)
        return 0;
    p = str;
    while (*p != '\0')  
        p++; 
    return p - str;
}

void serial_init(gpio_pin_t tx, gpio_pin_t rx, unsigned long baud){
    UART_BAUDRRATE = baud;
    UART_PSEL_TXD = (1UL & (tx.base == GPIO_P1_BASE)) << 5 | tx.pin;
    UART_PSEL_RXD = (1UL & (rx.base == GPIO_P1_BASE)) << 5 | rx.pin;
    UART_ENABLE = 8UL;
}

void serial_write(const char *buf, unsigned len){
    UART_TDX_PTR = (unsigned long)buf;
    UART_TDX_MAXCNT = len;
    UART_TASKS_STARTTX = 1; // Trigger sending task

    // wait for this transfer to finish
    while (UART_EVENTS_ENDTX == 0) { }
    UART_EVENTS_ENDTX = 0;              // clear
}

void serial_print(const char *str){
    unsigned len = strlen(str);

    if (len >= SERIAL_BUF_SIZE)
        return;

    for (unsigned i = 0; i < len; i++)
        serial_buf[i] = str[i];

    serial_write(serial_buf, len);
}

void serial_println(const char *str){
    serial_print(str);
    serial_print("\r\n");
}

char* itoa(unsigned long value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
  
    // Reverse the string
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

void serial_print_hex(unsigned long val){
    serial_buf[0] = '0';
    serial_buf[1] = 'x';
    itoa(val, &serial_buf[2], 16);

    serial_print(serial_buf);
}

void serial_print_dec(unsigned long val){
    itoa(val, serial_buf, 10);

    serial_print(serial_buf);
}