#ifndef PID_H
#define PID_H

#include <stdint.h>

/*
 * 位置式 PID。
 *   - 积分限幅（anti-windup）：积分项绝对值不超过 integral_limit。
 *   - 微分先行（derivative on measurement）：微分项基于测量值变化，
 *     避免设定值阶跃引起的微分冲击。
 */
typedef struct {
    float   kp;
    float   ki;
    float   kd;
    float   integral;
    float   integral_limit;
    float   output_limit;
    float   prev_measurement;
    uint8_t first;
} Pid;

void  Pid_Init(Pid *pid, float kp, float ki, float kd,
               float integral_limit, float output_limit);
float Pid_Update(Pid *pid, float setpoint, float measurement, float dt);
void  Pid_Reset(Pid *pid);

#endif /* PID_H */
