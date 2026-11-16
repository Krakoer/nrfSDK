#pragma once

#include <stdint.h>

typedef struct {
    uint8_t *buffer;            // pointer to externally allocated memory
    volatile uint32_t head;     // write counter
    volatile uint32_t tail;     // read counter
    uint32_t size;              // size of the ring
    uint32_t mask;              // mask to tranform head and tail counter into indexes
} ring_buff_t;

void ring_init(ring_buff_t *r, uint8_t *buf, uint32_t size);
uint32_t ring_write(ring_buff_t *r, const uint8_t *src, uint32_t len);
uint32_t ring_free(ring_buff_t *r);
void ring_peek(ring_buff_t *r, void **ptr, uint32_t *len);
void ring_consume(ring_buff_t *r, uint32_t n);
