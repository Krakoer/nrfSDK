#include "ring.h"
#include "string.h"
#include "hal.h"
#include "util.h"

/// Init the ring buffer.
/// The size MUST be a power of two.
void ring_init(ring_buff_t *r, uint8_t *buf, uint32_t size){
    r->buffer = buf;
    r->head = 0;
    r->tail = 0;
    r->size = size;
    r->mask = size-1;
}

/// Try to write len bytes from src in the ring buffer.
/// Returns number bytes written.
uint32_t ring_write(ring_buff_t *r, const uint8_t *src, uint32_t len){
    uint32_t available = ring_free(r);
    uint32_t to_write = (len < available) ? len : available;

    // Compute the amount of data we have from head to end of buffer.
    // first_write will be min(head_to_end, to_write)
    uint32_t first_write = r->size - (r->head & r->mask);
    if(first_write > to_write) first_write = to_write;

    // Perform the first write to the end of the buffer
    memcpy(&r->buffer[r->head&r->mask], src, first_write);

    // If we didn't write enough, wrap and write the second part
    if(first_write < to_write){
        uint32_t second_write = to_write - first_write;
        memcpy(r->buffer, &src[first_write], second_write);
    }

    // Memory barrier to make sure write occurs before advancing the head.
    __DMB();
    r->head += to_write;

    return to_write;
}

uint32_t ring_free(ring_buff_t *r){
    return r->size-(r->head-r->tail);
}

void ring_peek(ring_buff_t *r, void **ptr, uint32_t *len){
    uint32_t contig_data_available = MIN(r->head-r->tail, r->size-(r->tail&r->mask));
    *ptr = (long unsigned int *)&r->buffer[r->tail&r->mask];
    *len = contig_data_available;
}

void ring_consume(ring_buff_t *r, uint32_t n){
    __DMB();
    // Prevent ring corruption when n is greater than available data
    r->tail+=MIN(n, r->head-r->tail);
}