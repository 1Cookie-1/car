#include <stddef.h>

#include "scheduler.h"

#define SCHED_MAX_TASKS 8u

typedef struct {
    SchedTaskFn fn;
    uint16_t    period_ms;
    uint32_t    due;
} SchedTask;

static SchedTask s_tasks[SCHED_MAX_TASKS];
static uint8_t   s_task_count = 0;

void Sched_Init(void)
{
    s_task_count = 0;
}

bool Sched_Register(SchedTaskFn fn, uint16_t period_ms)
{
    uint8_t i;

    if (fn == NULL || period_ms == 0) {
        return false;
    }
    if (s_task_count >= SCHED_MAX_TASKS) {
        return false;
    }
    for (i = 0; i < s_task_count; i++) {
        if (s_tasks[i].fn == fn) {
            return false; /* 同一函数指针只能注册一次 */
        }
    }

    s_tasks[s_task_count].fn = fn;
    s_tasks[s_task_count].period_ms = period_ms;
    s_tasks[s_task_count].due = 0;
    s_task_count++;
    return true;
}

void Sched_Run(uint32_t now_ms)
{
    uint8_t i;

    /*
     * 每个任务的到期判断都直接使用当前传入的 now_ms，不缓存到函数级局部变量。
     * 若未来接入实时时间源，应在每个任务执行前重新读取当前时间，
     * 避免某个耗时任务影响后续任务的到期判断。
     */
    for (i = 0; i < s_task_count; i++) {
        if ((int32_t)(now_ms - s_tasks[i].due) >= 0) {
            s_tasks[i].due = now_ms + s_tasks[i].period_ms;
            s_tasks[i].fn();
        }
    }
}

uint8_t Sched_TaskCount(void)
{
    return s_task_count;
}
