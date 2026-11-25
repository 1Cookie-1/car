#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include "pid.h"

static int failures = 0;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static bool same_state(const Pid *a, const Pid *b)
{
    return a->kp == b->kp && a->ki == b->ki && a->kd == b->kd &&
           a->integral == b->integral && a->integral_limit == b->integral_limit &&
           a->output_limit == b->output_limit &&
           a->prev_measurement == b->prev_measurement && a->first == b->first;
}

int main(void)
{
    Pid pid;
    Pid saved;

    /* NULL pid 不崩溃，返回 0 */
    check(Pid_Update(NULL, 1.0f, 0.0f, 0.1f) == 0.0f, "NULL pid returns zero");
    Pid_Init(NULL, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f); /* 不应崩溃 */
    Pid_Reset(NULL);                              /* 不应崩溃 */

    /* 先积累历史，再验证非法输入不修改任何状态 */
    Pid_Init(&pid, 2.0f, 1.0f, 0.5f, 100.0f, 1000.0f);
    (void)Pid_Update(&pid, 10.0f, 2.0f, 0.5f);
    saved = pid;

    check(Pid_Update(&pid, 10.0f, 3.0f, NAN) == 0.0f, "NaN dt returns zero");
    check(same_state(&pid, &saved), "NaN dt preserves state");
    check(Pid_Update(&pid, 10.0f, 3.0f, INFINITY) == 0.0f, "+Inf dt returns zero");
    check(same_state(&pid, &saved), "+Inf dt preserves state");

    check(Pid_Update(&pid, NAN, 3.0f, 0.5f) == 0.0f, "NaN setpoint returns zero");
    check(same_state(&pid, &saved), "NaN setpoint preserves state");
    check(Pid_Update(&pid, 10.0f, INFINITY, 0.5f) == 0.0f,
          "Inf measurement returns zero");
    check(same_state(&pid, &saved), "Inf measurement preserves state");

    /* 非法输入之后，合法输入仍从原历史正常推进 */
    {
        Pid reference = saved;
        float expected = Pid_Update(&reference, 10.0f, 3.0f, 0.5f);
        float actual = Pid_Update(&pid, 10.0f, 3.0f, 0.5f);
        check(actual == expected && same_state(&pid, &reference),
              "valid dt resumes from unchanged history");
    }

    /* 非法限幅在 Init 时按 0 处理 */
    Pid_Init(&pid, 1.0f, 1.0f, 0.0f, -1.0f, 5.0f);
    check(pid.integral_limit == 0.0f, "negative integral limit sanitized to zero");
    check(pid.output_limit == 5.0f, "positive output limit kept");
    {
        float out = Pid_Update(&pid, 10.0f, 0.0f, 0.1f);
        check(pid.integral == 0.0f, "zero integral limit clamps integral to zero");
        check(out == 5.0f, "kp*error still limited by output limit");
    }

    /* NaN / Inf 增益与限幅在 Init 时按 0 处理 */
    Pid_Init(&pid, NAN, 1.0f, 0.5f, 100.0f, INFINITY);
    check(pid.kp == 0.0f, "NaN kp sanitized to zero");
    check(pid.ki == 1.0f, "finite ki kept");
    check(pid.kd == 0.5f, "finite kd kept");
    check(pid.integral_limit == 100.0f, "finite integral limit kept");
    check(pid.output_limit == 0.0f, "Inf output limit sanitized to zero");

    if (failures != 0) {
        return 1;
    }
    printf("pid_invalid_inputs: all checks passed\n");
    return 0;
}
