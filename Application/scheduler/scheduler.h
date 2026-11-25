#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*SchedTaskFn)(void);

/*
 * 注意：任务函数执行期间，注册表不可修改。
 * 即任务函数内部不得再调用 Sched_Init() 或 Sched_Register()。
 * 运行期间这两个调用会被拒绝：Sched_Register 返回 false，
 * Sched_Init 为无操作，以避免破坏 Sched_Run() 正在遍历的注册表。
 *
 * 并发与重入限制：
 *   - Sched_Run 不可嵌套：任务函数内部调用 Sched_Run 会被忽略（无操作）；
 *   - Sched_Init / Sched_Register / Sched_Run 均不是 ISR 安全的。
 *     s_run_depth 只用于阻止“同一执行上下文内”任务函数重入注册表，
 *     不提供任何原子性；中断服务程序中不得调用这些接口。
 *
 * 首次运行：任务注册后，在紧接着的第一次 Sched_Run 中立即执行一次，
 * 并以该时刻为基准锚定下一次到期时间。因此即使当前 tick 已经超过
 * 2^31，新注册任务也能正常触发（不会因 due=0 与回绕判断冲突而失火）。
 *
 * 调度策略：固定延时（fixed delay）。
 *   任务到期后，下一次到期时间按「当前传入时刻 + 周期」重新锚定，
 *   而不是按「上次到期时间 + 周期」推进。因此任务自身执行耗时、
 *   以及同一轮中排在前面的任务耗时，都会使后续任务的实际触发间隔
 *   大于设定周期，并产生累积漂移。需要固定频率（无漂移）的场合
 *   应另行评估调度策略。
 *
 * 时间回绕：Sched_Run 使用带符号差值判断到期，可正确处理 uint32_t
 * 毫秒时基的回绕，但两次到期判断之间的时间跨度必须小于 2^31 毫秒
 * （约 24.8 天）。
 */
void    Sched_Init(void);
bool    Sched_Register(SchedTaskFn fn, uint16_t period_ms);
void    Sched_Run(uint32_t now_ms);
uint8_t Sched_TaskCount(void);

#endif /* SCHEDULER_H */
