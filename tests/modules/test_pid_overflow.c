#include <stdbool.h>
#include <float.h>
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

static bool finite_val(float v)
{
    return (v >= -FLT_MAX) && (v <= FLT_MAX);
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
    float out;

    /* 1) 误差溢出：输入有限，setpoint - measurement 溢出为 Inf */
    Pid_Init(&pid, 1.0f, 0.5f, 0.0f, 1000.0f, 1000.0f);
    saved = pid;
    check(Pid_Update(&pid, FLT_MAX, -FLT_MAX, 0.1f) == 0.0f,
          "error overflow returns zero");
    check(same_state(&pid, &saved), "error overflow preserves all fields");

    /* 之后正常输入仍能算出有限值，且不越出输出限幅 */
    out = Pid_Update(&pid, 1.0f, 0.0f, 0.1f);
    check(finite_val(out), "normal input after error overflow yields finite output");
    check(out <= pid.output_limit && out >= -pid.output_limit,
          "output stays within configured limit");
    check(finite_val(pid.integral), "integral stays finite after recovery");

    /* 2) 积分增量溢出：ki * error * dt 溢出为 Inf */
    Pid_Init(&pid, 0.0f, FLT_MAX, 0.0f, 1e30f, 1e30f);
    saved = pid;
    check(Pid_Update(&pid, 2.0f, 0.0f, 1.0f) == 0.0f,
          "integral increment overflow returns zero");
    check(same_state(&pid, &saved), "integral overflow preserves all fields");

    /* 3) 微分项溢出：极小 dt 使 (prev - meas) / dt 溢出 */
    Pid_Init(&pid, 1.0f, 0.0f, FLT_MAX, 1000.0f, 1000.0f);
    (void)Pid_Update(&pid, 0.0f, 0.0f, 0.1f); /* 建立 prev_measurement 历史 */
    saved = pid;
    check(Pid_Update(&pid, 0.0f, 1.0f, 1e-38f) == 0.0f,
          "derivative overflow returns zero");
    check(same_state(&pid, &saved), "derivative overflow preserves all fields");

    /* 4) 输出溢出：kp * error 溢出为 Inf */
    Pid_Init(&pid, FLT_MAX, 0.0f, 0.0f, 1000.0f, 1000.0f);
    saved = pid;
    check(Pid_Update(&pid, 2.0f, 0.0f, 1.0f) == 0.0f,
          "output overflow returns zero");
    check(same_state(&pid, &saved), "output overflow preserves all fields");

    if (failures != 0) {
        return 1;
    }
    printf("pid_overflow: all checks passed\n");
    return 0;
}
