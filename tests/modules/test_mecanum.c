#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "mecanum.h"

static int failures = 0;

static void check(bool cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        failures++;
    }
}

static bool near(float a, float b)
{
    return fabsf(a - b) < 1e-4f;
}

int main(void)
{
    ChassisVelocity v;
    float w[MOTOR_COUNT];

    v.vx = 100.0f; v.vy = 0.0f; v.wz = 0.0f;
    Mecanum_Solve(&v, 150.0f, 120.0f, w);
    check(near(w[MOTOR_FL], 100.0f) && near(w[MOTOR_FR], 100.0f) &&
          near(w[MOTOR_RL], 100.0f) && near(w[MOTOR_RR], 100.0f),
          "case1: pure forward should give 100,100,100,100");

    v.vx = 0.0f; v.vy = 100.0f; v.wz = 0.0f;
    Mecanum_Solve(&v, 150.0f, 120.0f, w);
    check(near(w[MOTOR_FL], -100.0f) && near(w[MOTOR_FR], 100.0f) &&
          near(w[MOTOR_RL], 100.0f) && near(w[MOTOR_RR], -100.0f),
          "case2: pure strafe should give -100,100,100,-100");

    v.vx = 0.0f; v.vy = 0.0f; v.wz = 1000.0f;
    Mecanum_Solve(&v, 150.0f, 120.0f, w);
    check(near(w[MOTOR_FL], -270.0f) && near(w[MOTOR_FR], 270.0f) &&
          near(w[MOTOR_RL], -270.0f) && near(w[MOTOR_RR], 270.0f),
          "case3: pure rotation should give -270,270,-270,270");

    if (failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }

    printf("mecanum: all checks passed\n");
    return 0;
}
