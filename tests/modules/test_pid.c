#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "pid.h"

static int failures = 0;

static void check(bool cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        failures++;
    }
}

int main(void)
{
    Pid pid;
    float meas;
    float out;
    float max_meas;
    int i;

    /* 1) 阶跃跟踪收敛 */
    Pid_Init(&pid, 1.0f, 0.5f, 0.0f, 1000.0f, 1000.0f);
    meas = 0.0f;
    for (i = 0; i < 3000; i++) {
        out = Pid_Update(&pid, 10.0f, meas, 0.01f);
        meas += out * 0.01f;
    }
    check(fabsf(meas - 10.0f) < 0.1f, "step response should converge near setpoint");

    /* 2) 输出限幅 */
    Pid_Init(&pid, 100.0f, 0.0f, 0.0f, 1000.0f, 5.0f);
    out = Pid_Update(&pid, 100.0f, 0.0f, 0.01f);
    check(fabsf(out - 5.0f) < 1e-4f, "output should clamp to +limit");

    /* 3) 积分限幅 + 饱和后不超调失控 */
    Pid_Init(&pid, 1.0f, 10.0f, 0.0f, 2.0f, 5.0f);
    meas = 0.0f;
    max_meas = 0.0f;
    {
        bool bounded = true;
        for (i = 0; i < 3000; i++) {
            out = Pid_Update(&pid, 10.0f, meas, 0.01f);
            if (out < -5.0f || out > 5.0f) {
                bounded = false;
            }
            if (pid.integral < -2.0f || pid.integral > 2.0f) {
                bounded = false;
            }
            meas += out * 0.01f;
            if (meas > max_meas) {
                max_meas = meas;
            }
        }
        check(bounded, "output and integral stay within limits during saturation");
    }
    check(max_meas < 20.0f, "no overshoot runaway after integral saturation");
    check(meas > -20.0f && meas < 20.0f, "plant stays bounded");

    if (failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }

    printf("pid: all checks passed\n");
    return 0;
}
