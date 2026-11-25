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

    /* NULL 指针不崩溃 */
    Mecanum_Solve(NULL, 150.0f, 120.0f, w);
    Mecanum_Solve(&v, 150.0f, 120.0f, NULL);

    /* NULL v：输出数组保持不变，调用方不得当作新结果 */
    {
        float sentinel[MOTOR_COUNT] = {1234.0f, 1234.0f, 1234.0f, 1234.0f};
        Mecanum_Solve(NULL, 150.0f, 120.0f, sentinel);
        check(sentinel[0] == 1234.0f && sentinel[1] == 1234.0f &&
              sentinel[2] == 1234.0f && sentinel[3] == 1234.0f,
              "NULL v leaves output array unchanged");
    }

    /* 复合运动：vx=100, vy=50, wz=500, half_len=150, half_wid=120 */
    v.vx = 100.0f;
    v.vy = 50.0f;
    v.wz = 500.0f;
    Mecanum_Solve(&v, 150.0f, 120.0f, w);
    /* term = (150 + 120) * 500 / 1000 = 135 */
    check(near(w[MOTOR_FL], 100.0f - 50.0f - 135.0f) &&
          near(w[MOTOR_FR], 100.0f + 50.0f + 135.0f) &&
          near(w[MOTOR_RL], 100.0f + 50.0f - 135.0f) &&
          near(w[MOTOR_RR], 100.0f - 50.0f + 135.0f),
          "mixed motion expected values");

    /* 数值复核：输出是 mm/s 线速度，与轮子半径无关 */
    check(near(w[MOTOR_FL], -85.0f) && near(w[MOTOR_FR], 285.0f) &&
          near(w[MOTOR_RL], 15.0f) && near(w[MOTOR_RR], 185.0f),
          "mixed motion numeric values");

    if (failures != 0) {
        return 1;
    }
    printf("mecanum_extra: all checks passed\n");
    return 0;
}
