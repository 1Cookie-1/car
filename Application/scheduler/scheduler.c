#include <stddef.h>

#include "scheduler.h"

#define SCHED_MAX_TASKS 8u

typedef struct {
    SchedTaskFn fn;
    uint16_t    period_ms;
    uint32_t    due;
    bool        scheduled;   /* 是否已经历过首次运行 */
} SchedTask;

static SchedTask s_tasks[SCHED_MAX_TASKS];
static uint8_t   s_task_count = 0;
static uint8_t   s_run_depth  = 0;

void Sched_Init(void)
{
    if (s_run_depth != 0u) {
        return; /* 运行期间禁止重置任务表 */
    }
    s_task_count = 0;
}

bool Sched_Register(SchedTaskFn fn, uint16_t period_ms)
{
    uint8_t i;

    if (s_run_depth != 0u) {
        return false; /* 任务执行期间注册表不可修改 */
    }
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
    s_tasks[s_task_count].scheduled = false;
    s_task_count++;
    return true;
}

void Sched_Run(uint32_t now_ms)
{
    uint8_t i;

    if (s_run_depth != 0u) {
        return; /* 禁止嵌套调用 Sched_Run */
    }

    s_run_depth = 1u;

    /*
     * 固定延时策略：每个任务的到期判断都使用本函数收到的 now_ms，
     * 到期后按 now_ms + period 重新锚定下一次到期时间。前一个任务执行
     * 耗时过长会推迟后续任务的触发并产生漂移，因此任务函数必须短小、
     * 不得长时间阻塞。
     */
    for (i = 0; i < s_task_count; i++) {
        bool fire = false;

        if (!s_tasks[i].scheduled) {
            /* 首次运行：立即执行，并以此刻为基准锚定下一次到期时间，
               避免 tick 已超过 2^31 时 due=0 无法判定的问题。 */
            s_tasks[i].scheduled = true;
            s_tasks[i].due = now_ms + s_tasks[i].period_ms;
            fire = true;
        } else if ((int32_t)(now_ms - s_tasks[i].due) >= 0) {
            s_tasks[i].due = now_ms + s_tasks[i].period_ms;
            fire = true;
        }

        if (fire) {
            s_tasks[i].fn();
        }
    }

    s_run_depth = 0u;
}

uint8_t Sched_TaskCount(void)
{
    return s_task_count;
}
