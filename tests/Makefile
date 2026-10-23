CC = gcc
CFLAGS = -Wall -Wextra -I../include -fsanitize=address,undefined
SRCS = $(wildcard ./*.c) ../src/ble_ad.c

check: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o test && ./test && rm test