#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ring_buffer.h"

#define TEST_STORAGE_SIZE 4u

static int failures = 0;

static void check(bool cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        failures++;
    }
}

int main(void)
{
    uint8_t storage[TEST_STORAGE_SIZE];
    RingBuffer rb;
    uint8_t byte;
    bool ok;

    RingBuffer_Init(&rb, storage, TEST_STORAGE_SIZE);

    /* 初始化状态 */
    check(RingBuffer_IsEmpty(&rb), "init: should be empty");
    check(!RingBuffer_IsFull(&rb), "init: should not be full");
    check(RingBuffer_Count(&rb) == 0u, "init: count should be 0");
    check(!RingBuffer_Get(&rb, &byte), "empty buffer Get should return false");

    /* 填满：size=4，可用容量为 3 */
    check(RingBuffer_Put(&rb, 0x11), "put 0x11");
    check(RingBuffer_Put(&rb, 0x22), "put 0x22");
    check(RingBuffer_Put(&rb, 0x33), "put 0x33");
    check(RingBuffer_IsFull(&rb), "should be full after 3 puts");
    check(RingBuffer_Count(&rb) == 3u, "count should be 3");
    check(!RingBuffer_Put(&rb, 0x44), "put into full buffer should return false");

    /* 取出顺序 */
    check(RingBuffer_Get(&rb, &byte) && byte == 0x11, "read 0x11");
    check(RingBuffer_Get(&rb, &byte) && byte == 0x22, "read 0x22");
    check(RingBuffer_Get(&rb, &byte) && byte == 0x33, "read 0x33");
    check(RingBuffer_IsEmpty(&rb), "should be empty after reading all");
    check(!RingBuffer_Get(&rb, &byte), "empty buffer Get should return false again");

    /* 绕回：累计写入超过 size 的字节，取出顺序仍正确 */
    ok = RingBuffer_Put(&rb, 0x44);
    ok = RingBuffer_Put(&rb, 0x55) && ok;
    ok = RingBuffer_Put(&rb, 0x66) && ok;
    check(ok, "wraparound puts 0x44,0x55,0x66");
    check(!RingBuffer_Put(&rb, 0x77), "wraparound full should reject 0x77");

    check(RingBuffer_Get(&rb, &byte) && byte == 0x44, "wraparound read 0x44");
    check(RingBuffer_Get(&rb, &byte) && byte == 0x55, "wraparound read 0x55");
    check(RingBuffer_Get(&rb, &byte) && byte == 0x66, "wraparound read 0x66");
    check(RingBuffer_IsEmpty(&rb), "empty after wraparound reads");

    if (failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }

    printf("ring_buffer: all checks passed\n");
    return 0;
}
