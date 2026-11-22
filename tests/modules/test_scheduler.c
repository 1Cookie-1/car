#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "scheduler.h"

static int failures = 0;

static void check(bool cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        failures++;
    }
}

static int cnt_10;
static int cnt_50;

static void task_10(void) { cnt_10++; }
static void task_50(void) { cnt_50++; }

static void t0(void) {}
static void t1(void) {}
static void t2(void) {}
static void t3(void) {}
static void t4(void) {}
static void t5(void) {}
static void t6(void) {}
static void t7(void) {}
static void t8(void) {}

int main(void)
{
    int i;

    /* 1) 基本周期调度 */
    Sched_Init();
    cnt_10 = 0;
    cnt_50 = 0;
    check(Sched_Register(task_10, 10), "register 10ms");
    check(Sched_Register(task_50, 50), "register 50ms");
    check(Sched_TaskCount() == 2, "task count should be 2");
    check(!Sched_Register(task_10, 10), "duplicate register should fail");
    check(Sched_TaskCount() == 2, "task count still 2 after duplicate");

    Sched_Run(0);
    Sched_Run(10);
    Sched_Run(11);
    Sched_Run(20);
    Sched_Run(50);
    Sched_Run(60);
    check(cnt_10 == 5, "10ms task should run 5 times");
    check(cnt_50 == 2, "50ms task should run 2 times");

    /* 2) 溢出：due 推进到 0xFFFFF000 附近，now 回绕后仍应到期 */
    Sched_Init();
    cnt_10 = 0;
    check(Sched_Register(task_10, 1000), "register overflow task");
    Sched_Run(0x7FFFFFFFu);   /* due = 0x7FFFFFFF + 1000 = 0x800003E7 */
    Sched_Run(0xFFFFEC18u);   /* due = 0xFFFFEC18 + 1000 = 0xFFFFF000 */
    {
        int before = cnt_10;
        Sched_Run(0x00000FF0u); /* now 回绕，仍应判定为到期 */
        check(cnt_10 == before + 1, "task should fire after uint32 wrap");
    }

    /* 3) 最多 8 个任务 */
    Sched_Init();
    {
        SchedTaskFn fns[9] = { t0, t1, t2, t3, t4, t5, t6, t7, t8 };
        int ok = 0;
        for (i = 0; i < 8; i++) {
            if (Sched_Register(fns[i], 1)) {
                ok++;
            }
        }
        check(ok == 8, "first 8 registrations should succeed");
        check(Sched_TaskCount() == 8, "task count should be 8");
        check(!Sched_Register(fns[8], 1), "9th registration should fail");
        check(Sched_TaskCount() == 8, "task count stays 8");
    }

    /* 4) period_ms == 0 应失败 */
    Sched_Init();
    check(!Sched_Register(t0, 0), "period 0 should fail");
    check(Sched_TaskCount() == 0, "task count should stay 0");

    if (failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }

    printf("scheduler: all checks passed\n");
    return 0;
}
