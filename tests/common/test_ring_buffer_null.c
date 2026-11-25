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
    uint8_t storage[2];
    uint8_t byte;
    RingBuffer rb;

    /* NULL rb：所有查询接口不崩溃且返回一致结果 */
    RingBuffer_Init(NULL, storage, 2u);
    check(!RingBuffer_Put(NULL, 0x11u), "NULL rb rejects Put");
    check(!RingBuffer_Get(NULL, &byte), "NULL rb rejects Get");
    check(RingBuffer_Count(NULL) == 0u, "NULL rb count is zero");
    check(RingBuffer_IsEmpty(NULL), "NULL rb is empty");
    check(!RingBuffer_IsFull(NULL), "NULL rb is not full");

    /* NULL 存储区：初始化为无效缓冲区 */
    RingBuffer_Init(&rb, NULL, 2u);
    check(rb.size == 0u, "NULL storage yields invalid buffer");
    check(!RingBuffer_Put(&rb, 0x11u), "NULL storage rejects Put");
    check(!RingBuffer_Get(&rb, &byte), "NULL storage rejects Get");
    check(RingBuffer_Count(&rb) == 0u, "NULL storage count is zero");
    check(RingBuffer_IsEmpty(&rb), "NULL storage is empty");
    check(!RingBuffer_IsFull(&rb), "NULL storage is not full");

    /* NULL 输出指针：不崩溃、不消费数据 */
    RingBuffer_Init(&rb, storage, 2u);
    check(RingBuffer_Put(&rb, 0xAAu), "valid buffer accepts Put");
    check(!RingBuffer_Get(&rb, NULL), "NULL out rejects Get");
    check(RingBuffer_Count(&rb) == 1u, "rejected Get does not consume byte");
    check(RingBuffer_Get(&rb, &byte) && byte == 0xAAu,
          "byte still readable after rejected Get");

    if (failures != 0) {
        return 1;
    }
    printf("ring_buffer_null: all checks passed\n");
    return 0;
}
