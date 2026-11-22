#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * 环形缓冲区（单生产者 / 单消费者）。
 *
 * 使用前提：
 *   - 同一时刻最多一个生产者调用 RingBuffer_Put；
 *   - 同一时刻最多一个消费者调用 RingBuffer_Get。
 * 本实现不提供锁或临界区保护，不适用于多生产者或多消费者并发场景。
 *
 * 约定：
 *   - head == tail 表示缓冲区为空；
 *   - 始终保留一个空位用于区分空/满，有效容量 = size - 1，最小 size 为 2；
 *   - size 不要求是 2 的幂；size < 2 时初始化为无效缓冲区；
 *   - 无效缓冲区 Put/Get 返回 false，Count 返回 0，IsEmpty 为 true，IsFull 为 false；
 *   - 不使用动态内存，存储空间由调用者提供。
 */
typedef struct {
    uint8_t          *buf;
    uint16_t          size;
    volatile uint16_t head;   /* 写入位置 */
    volatile uint16_t tail;   /* 读取位置 */
} RingBuffer;

void     RingBuffer_Init(RingBuffer *rb, uint8_t *storage, uint16_t size);
bool     RingBuffer_Put(RingBuffer *rb, uint8_t byte);   /* 满返回 false */
bool     RingBuffer_Get(RingBuffer *rb, uint8_t *byte);  /* 空返回 false */
uint16_t RingBuffer_Count(const RingBuffer *rb);
bool     RingBuffer_IsEmpty(const RingBuffer *rb);
bool     RingBuffer_IsFull(const RingBuffer *rb);

#endif /* RING_BUFFER_H */
