#include "serial.h"
#include "nrf52_regs.h"
#include <stdint.h>
#include "string.h"

#define SERIAL_BUF_SIZE 256

static char serial_buf[SERIAL_BUF_SIZE] = {0};

void serial_print_hex8(uint8_t val) {
    char buf[2];
    hex8(buf, val);
    serial_write(buf, sizeof buf);
}

void serial_print_hex32(uint32_t val) {
    char buf[10];
    hex32(buf, val);
    serial_write(buf, 10);
}

void serial_print_dec(unsigned long v) {
    char buf[10];
    unsigned len;
    char *p = utoa_back(v, buf + sizeof buf, &len);
    serial_write(p, len);
}

void serial_hex_dump(const uint8_t *buf, unsigned len){
    char formatted[SERIAL_BUF_SIZE] = {0};
    unsigned int written = 0;
    unsigned int taken = 0;
    unsigned int total = 0;

    while(1) {
        taken = hexdump(&buf[total], len-total, formatted, SERIAL_BUF_SIZE, &written, total);
        if(!taken) {
            break;
        }
        total += taken;
        serial_write(formatted, written);
        if(total == len){
            break;
        }
    }
}

void serial_init(gpio_pin_t tx, gpio_pin_t rx, unsigned long baud){
    UART_BAUDRRATE = baud;
    UART_PSEL_TXD = (1UL & (tx.base == GPIO_P1_BASE)) << 5 | tx.pin;
    UART_PSEL_RXD = (1UL & (rx.base == GPIO_P1_BASE)) << 5 | rx.pin;
    UART_ENABLE = 8UL;
}

void serial_write(const char *buf, unsigned len){
    UART_TDX_PTR = (unsigned long)buf;
    // Tells the compiler that caller to this function
    // might have allocated memory used here, so don't
    // optimize memory writes before a call to serial_write.
    __asm__ volatile ("" ::: "memory");
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
