#ifndef PID_H
#define PID_H

#include <stdint.h>

/*
 * 位置式 PID。
 *   - 积分限幅（anti-windup）：积分项绝对值不超过 integral_limit。
 *   - 微分先行（derivative on measurement）：微分项基于测量值变化，
 *     避免设定值阶跃引起的微分冲击。
 *
 * 非法输入行为：
 *   - pid 为 NULL：Init / Reset 直接返回，Update 返回 0.0f，不访问状态；
 *   - dt 必须是有限正数：dt <= 0、NaN 或 +Inf 时 Update 返回 0.0f，
 *     且不修改 PID 内部任何状态；
 *   - setpoint / measurement 为 NaN 或 Inf 时 Update 返回 0.0f，
 *     且不修改 PID 内部任何状态；
 *   - Update 采用事务式提交：误差、积分增量、积分候选值、微分项、
 *     最终输出中任一步因有限输入溢出为 Inf/NaN 时，返回 0.0f 且
 *     不修改 integral / prev_measurement / first 等任何内部状态；
 *   - 非法增益（NaN、Inf）在 Init 时按 0.0f 处理；
 *   - 非法限幅（负值、NaN、Inf）在 Init 时按 0.0f 处理，
 *     即对应项被钳位为 0。
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
