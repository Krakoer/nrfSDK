#pragma once

#include "gpio.h"

#define BAUD1200 0x0004F000 // 1200 baud (actual rate: 1205)
#define BAUD2400 0x0009D000 // 2400 baud (actual rate: 2396)
#define BAUD4800 0x0013B000 // 4800 baud (actual rate: 4808)
#define BAUD9600 0x00275000 // 9600 baud (actual rate: 9598)
#define BAUD14400 0x003B0000 // 14400 baud (actual rate: 14414)
#define BAUD19200 0x004EA000 // 19200 baud (actual rate: 19208)
#define BAUD28800 0x0075F000 // 28800 baud (actual rate: 28829)
#define BAUD31250 0x00800000 // 31250 baud
#define BAUD38400 0x009D5000 // 38400 baud (actual rate: 38462)
#define BAUD56000 0x00E50000 // 56000 baud (actual rate: 55944)
#define BAUD57600 0x00EBF000 // 57600 baud (actual rate: 57762)
#define BAUD76800 0x013A9000 // 76800 baud (actual rate: 76923)
#define BAUD115200 0x01D7E000 // 115200 baud (actual rate: 115942)
#define BAUD230400 0x03AFB000 // 230400 baud (actual rate: 231884)
#define BAUD250000 0x04000000 // 250000 baud
#define BAUD460800 0x075F7000 // 460800 baud (actual rate: 470588)
#define BAUD921600 0x0EBED000 // 921600 baud (actual rate: 941176)
#define BAUD1M 0x10000000 // 1Mega baud

void serial_init(gpio_pin_t tx, gpio_pin_t rx, unsigned long baud);
void serial_write(const char *buf, unsigned len);
void serial_print(const char *str);
void serial_println(const char *str);
void serial_print_dec(unsigned long val);
void serial_print_hex(unsigned long val);