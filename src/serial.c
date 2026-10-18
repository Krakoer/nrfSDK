#include "serial.h"
#include "nrf52_regs.h"
#include <stdint.h>

#define HEXDUMP_COL  16
#define HEXDUMP_LINE (6 + 3*HEXDUMP_COL + 2)

static const char hex_digits[] = "0123456789ABCDEF";

#define SERIAL_BUF_SIZE 256

static char serial_buf[SERIAL_BUF_SIZE] = {0};

static inline void hex8(char* out, uint8_t b) {
    out[0] = hex_digits[(b >> 4) & 0xF];
    out[1] = hex_digits[b & 0xF];
}

void serial_print_hex8(uint8_t val) {
    char buf[2];
    hex8(buf, val);
    serial_write(buf, sizeof buf);
}

void serial_print_hex32(uint32_t val) {
    char buf[2 + 8];
    buf[0] = '0';
    buf[1] = 'x';

    for(int i = 0; i < 4; i++){
        hex8(&buf[2+2*i], (val >> 8*(3-i)) & 0xFF);
    }

    serial_write(buf, 10);
}

static char *utoa_back(unsigned long v, char *end, unsigned *len)
{
    char *p = end;
    do { *--p = (char)('0' + (v % 10UL)); v /= 10UL; } while (v);
    *len = (unsigned)(end - p);
    return p;
}

void serial_print_dec(unsigned long v)
{
    char buf[10];
    unsigned len;
    char *p = utoa_back(v, buf + sizeof buf, &len);
    serial_write(p, len);
}

void serial_hex_dump(const uint8_t *buf, unsigned len){
    char line[HEXDUMP_LINE] = {0};

    for(int off = 0; off < len; off+=HEXDUMP_COL) {
        // Number of byte to print in this line
        int n = len-off;
        if(n > HEXDUMP_COL) n = HEXDUMP_COL;
        
        // Offset: '00EF: '
        hex8(line, (off >> 16) & 0xFF);
        hex8(&line[2], off & 0xFF);
        line[4] = ':';
        line[5] = ' ';


        for(int i = 0; i < n; i++){
            hex8(&line[6+i*3], buf[off+i]);
            line[6+i*3+2] = ' ';
        }

        line[6+n*3] = '\r';
        line[6+n*3+1] = '\n';
        serial_write(line, 6+n*3+2);
    }
}

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
