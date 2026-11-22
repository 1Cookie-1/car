#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ring_buffer.h"

static int failures = 0;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    uint8_t storage[2] = {0x11u, 0x22u};
    RingBuffer rb;
    uint8_t byte;

    for (uint16_t size = 0; size < 2u; size++) {
        RingBuffer_Init(&rb, storage, size);
        byte = 0xA5u;
        check(!RingBuffer_Put(&rb, 0x33u), "invalid size rejects Put");
        check(!RingBuffer_Get(&rb, &byte), "invalid size rejects Get");
        check(byte == 0xA5u, "rejected Get leaves output unchanged");
        check(storage[0] == 0x11u && storage[1] == 0x22u,
              "rejected Put leaves storage unchanged");
        check(rb.head == 0u && rb.tail == 0u, "invalid buffer indices stay zero");
        check(RingBuffer_Count(&rb) == 0u, "invalid buffer count is zero");
        check(RingBuffer_IsEmpty(&rb), "invalid buffer is empty");
        check(!RingBuffer_IsFull(&rb), "invalid buffer is not full");
    }

    RingBuffer_Init(&rb, storage, 2u);
    check(RingBuffer_IsEmpty(&rb) && !RingBuffer_IsFull(&rb), "size 2 starts empty");
    for (uint8_t value = 1; value <= 2u; value++) {
        check(RingBuffer_Put(&rb, value), "size 2 accepts one byte");
        check(RingBuffer_Count(&rb) == 1u && RingBuffer_IsFull(&rb),
              "size 2 has capacity one");
        check(!RingBuffer_Put(&rb, 0xFFu), "size 2 rejects a second byte");
        check(RingBuffer_Get(&rb, &byte) && byte == value, "size 2 reads across wrap");
        check(RingBuffer_Count(&rb) == 0u && RingBuffer_IsEmpty(&rb),
              "size 2 is empty after Get");
    }

    if (failures != 0) {
        return 1;
    }
    printf("ring_buffer_invalid_size: all checks passed\n");
    return 0;
}
