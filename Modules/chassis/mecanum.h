#ifndef MECANUM_H
#define MECANUM_H

#include "motor_if.h"

typedef struct { float vx; float vy; float wz; } ChassisVelocity;
/* vx 前后 mm/s，vy 左右 mm/s（左为正），wz 自转 mrad/s（逆时针为正）*/

void Mecanum_Solve(const ChassisVelocity *v, float half_len_mm, float half_wid_mm,
                   float wheel_mm_s[MOTOR_COUNT]);

#endif /* MECANUM_H */
