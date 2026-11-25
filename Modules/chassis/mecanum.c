#include <stddef.h>

#include "mecanum.h"

void Mecanum_Solve(const ChassisVelocity *v, float half_len_mm, float half_wid_mm,
                   float wheel_mm_s[MOTOR_COUNT])
{
    float term;

    if (v == NULL || wheel_mm_s == NULL) {
        return;
    }

    term = (half_len_mm + half_wid_mm) * v->wz / 1000.0f;

    wheel_mm_s[MOTOR_FL] = v->vx - v->vy - term;
    wheel_mm_s[MOTOR_FR] = v->vx + v->vy + term;
    wheel_mm_s[MOTOR_RL] = v->vx + v->vy - term;
    wheel_mm_s[MOTOR_RR] = v->vx - v->vy + term;
}
