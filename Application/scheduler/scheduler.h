#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*SchedTaskFn)(void);

/*
 * 注意：任务函数执行期间，注册表不可修改。
 * 即任务函数内部不得再调用 Sched_Init() 或 Sched_Register()，
 * 否则会破坏 Sched_Run() 正在遍历的注册表。
 */
void    Sched_Init(void);
bool    Sched_Register(SchedTaskFn fn, uint16_t period_ms);
void    Sched_Run(uint32_t now_ms);
uint8_t Sched_TaskCount(void);

#endif /* SCHEDULER_H */
