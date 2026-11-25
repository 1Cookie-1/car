#include <stddef.h>

#include "ring_buffer.h"

void RingBuffer_Init(RingBuffer *rb, uint8_t *storage, uint16_t size)
{
    if (rb == NULL) {
        return;
    }

    rb->buf = storage;
    if ((size >= 2u) && (storage != NULL)) {
        rb->size = size;
    } else {
        rb->size = 0u;
    }
    rb->head = 0;
    rb->tail = 0;
}

bool RingBuffer_Put(RingBuffer *rb, uint8_t byte)
{
    uint16_t next;

    if (rb == NULL || rb->buf == NULL || rb->size < 2u) {
        return false;
    }
    next = (uint16_t)((rb->head + 1u) % rb->size);

    if (next == rb->tail) {
        return false; /* 缓冲区已满 */
    }

    rb->buf[rb->head] = byte;
    rb->head = next;
    return true;
}

bool RingBuffer_Get(RingBuffer *rb, uint8_t *byte)
{
    if (rb == NULL || byte == NULL || rb->buf == NULL || rb->size < 2u) {
        return false;
    }
    if (rb->head == rb->tail) {
        return false; /* 缓冲区已空 */
    }

    *byte = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1u) % rb->size);
    return true;
}

uint16_t RingBuffer_Count(const RingBuffer *rb)
{
    if (rb == NULL || rb->size < 2u) {
        return 0;
    }
    return (uint16_t)(((uint32_t)rb->head + rb->size - rb->tail) % rb->size);
}

bool RingBuffer_IsEmpty(const RingBuffer *rb)
{
    if (rb == NULL || rb->size < 2u) {
        return true;
    }
    return rb->head == rb->tail;
}

bool RingBuffer_IsFull(const RingBuffer *rb)
{
    uint16_t next;

    if (rb == NULL || rb->size < 2u) {
        return false;
    }
    next = (uint16_t)((rb->head + 1u) % rb->size);
    return next == rb->tail;
}
