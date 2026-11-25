#ifndef MECANUM_H
#define MECANUM_H

#include "motor_if.h"

typedef struct { float vx; float vy; float wz; } ChassisVelocity;

/*
 * 麦轮逆运动学解算（纯数学，不依赖 HAL）。
 *
 * 输入：
 *   v->vx：前后方向线速度，单位 mm/s，前进为正；
 *   v->vy：左右方向线速度，单位 mm/s，左移为正；
 *   v->wz：绕底盘中心自转角速度，单位 mrad/s，逆时针为正；
 *   half_len_mm：前后半轴距（中心到前后轮轴线距离），单位 mm，必须为正；
 *   half_wid_mm：左右半轮距（中心到左右轮轴线距离），单位 mm，必须为正。
 *
 * 输出：
 *   wheel_mm_s[MOTOR_COUNT]：各轮目标线速度，单位 mm/s。只给轮面线速度，
 *   不涉及轮子半径；由上层结合轮径、细分等换算为电机转速或脉冲频率。
 *
 * 车轮顺序：MOTOR_FL 左前、MOTOR_FR 右前、MOTOR_RL 左后、MOTOR_RR 右后。
 * 公式与旋转方向约定由 test_mecanum 固定；实际轮子安装方向与电机正负
 * 方向留待硬件到货后验证，不在本模块中擅自确定。
 *
 * 非法几何参数：half_len_mm / half_wid_mm 必须为正有限数，由调用方保证。
 *   本函数不检查、也无错误返回；传入 0、负数、NaN 或 Inf 时输出无定义
 *   （垃圾进垃圾出），责任在调用方。
 *
 * 非法输入：v->vx / vy / wz 为 NaN 或 Inf 时输出为 NaN / Inf
 *   （纯数学函数，垃圾进垃圾出）。
 *
 * NULL 输入：v 或 wheel_mm_s 为 NULL 时函数直接返回、不写任何数据；
 *   此时输出数组保持原值，调用方不得将其当作新得到的有效轮速。
 */
void Mecanum_Solve(const ChassisVelocity *v, float half_len_mm, float half_wid_mm,
                   float wheel_mm_s[MOTOR_COUNT]);

#endif /* MECANUM_H */
