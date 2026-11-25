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

static void t0(void) {}
static void t_extra(void) {}

static int order[8];
static int order_cnt;
static int normal_cnt;
static int guard_called;
static int guard_result;
static int big_cnt;
static int nested_guard;
static int nested_called;
static int partner_cnt;

static void rec0(void) { order[order_cnt++] = 0; }
static void rec1(void) { order[order_cnt++] = 1; }
static void rec2(void) { order[order_cnt++] = 2; }

static void normal_task(void) { normal_cnt++; }
static void task_big(void) { big_cnt++; }

static void partner_task(void) { partner_cnt++; }

static void nested_runner(void)
{
    if (!nested_guard) {
        nested_guard = 1;
        Sched_Run(999); /* 嵌套调用应被忽略 */
    }
    nested_called++;
}

static void guarded_task(void)
{
    if (!guard_called) {
        guard_called = 1;
        guard_result = Sched_Register(t_extra, 10); /* 运行中注册应被拒绝 */
        Sched_Init();                               /* 运行中重置应为无操作 */
    }
}

int main(void)
{
    /* 1) NULL 函数指针被拒绝 */
    Sched_Init();
    check(!Sched_Register(NULL, 10), "NULL fn rejected");
    check(Sched_TaskCount() == 0, "NULL fn leaves table empty");

    /* 2) 任务按注册顺序执行 */
    Sched_Init();
    order_cnt = 0;
    check(Sched_Register(rec0, 1), "register rec0");
    check(Sched_Register(rec1, 1), "register rec1");
    check(Sched_Register(rec2, 1), "register rec2");
    Sched_Run(0);
    check(order_cnt == 3, "three tasks fired in one run");
    check(order[0] == 0 && order[1] == 1 && order[2] == 2,
          "execution order equals registration order");

    /* 3) 运行期间修改任务表被拒绝，且表不被破坏 */
    Sched_Init();
    guard_called = 0;
    guard_result = 1;
    normal_cnt = 0;
    check(Sched_Register(guarded_task, 1), "register guarded task");
    check(Sched_Register(normal_task, 1), "register normal task");
    check(Sched_TaskCount() == 2, "two tasks registered");
    Sched_Run(0);
    check(guard_called == 1, "guarded task fired");
    check(guard_result == 0, "register during run rejected");
    check(Sched_TaskCount() == 2, "task table unchanged after run");
    check(normal_cnt == 1, "normal task still fired in same run");
    Sched_Run(1);
    check(normal_cnt == 2, "normal task still scheduled after guarded run");

    /* 4) 最大周期合法 */
    Sched_Init();
    check(Sched_Register(t0, 65535), "max uint16 period accepted");

    /* 5) tick 已超过 2^31 时注册，首次运行仍应立即触发 */
    Sched_Init();
    big_cnt = 0;
    check(Sched_Register(task_big, 1000), "register task before big tick");
    Sched_Run(0x80000000u);
    check(big_cnt == 1, "task fires immediately when tick >= 2^31");
    Sched_Run(0x80000001u);
    check(big_cnt == 1, "task does not fire before period");
    Sched_Run(0x800003E8u); /* 0x80000000 + 1000 */
    check(big_cnt == 2, "task fires again after one period");

    /* 6) 嵌套 Sched_Run 被忽略，不产生重入触发 */
    Sched_Init();
    nested_guard = 0;
    nested_called = 0;
    partner_cnt = 0;
    check(Sched_Register(nested_runner, 1), "register nested runner");
    check(Sched_Register(partner_task, 1), "register partner task");
    Sched_Run(0);
    check(nested_called == 1, "nested Sched_Run is ignored (runner fires once)");
    check(partner_cnt == 1, "partner task fires in same run");
    Sched_Run(1);
    check(nested_called == 2, "runner fires again next tick without recursion");
    check(partner_cnt == 2, "partner task fires again next tick");

    if (failures != 0) {
        return 1;
    }
    printf("scheduler_extra: all checks passed\n");
    return 0;
}
