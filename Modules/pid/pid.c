#include <stdbool.h>
#include <stddef.h>
#include <float.h>

#include "pid.h"

static bool pid_is_finite(float v)
{
    return (v >= -FLT_MAX) && (v <= FLT_MAX);
}

static float clampf(float v, float limit)
{
    /* limit <= 0 或 NaN 时按钳位到 0 处理 */
    if (!(limit > 0.0f)) {
        return 0.0f;
    }
    if (v > limit) {
        return limit;
    }
    if (v < -limit) {
        return -limit;
    }
    return v;
}

static float sanitize_gain(float gain)
{
    return pid_is_finite(gain) ? gain : 0.0f;
}

static float sanitize_limit(float limit)
{
    /* 限幅是幅值（钳位到 [-limit, +limit]），必须为正有限数 */
    return (pid_is_finite(limit) && (limit > 0.0f)) ? limit : 0.0f;
}

void Pid_Init(Pid *pid, float kp, float ki, float kd,
              float integral_limit, float output_limit)
{
    if (pid == NULL) {
        return;
    }

    pid->kp = sanitize_gain(kp);
    pid->ki = sanitize_gain(ki);
    pid->kd = sanitize_gain(kd);
    pid->integral = 0.0f;
    pid->integral_limit = sanitize_limit(integral_limit);
    pid->output_limit = sanitize_limit(output_limit);
    pid->prev_measurement = 0.0f;
    pid->first = 1;
}

void Pid_Reset(Pid *pid)
{
    if (pid == NULL) {
        return;
    }
    pid->integral = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->first = 1;
}

float Pid_Update(Pid *pid, float setpoint, float measurement, float dt)
{
    float error;
    float increment;
    float new_integral;
    float d_term;
    float out;

    if (pid == NULL) {
        return 0.0f;
    }

    /* dt 必须是有限正数：拒绝 0、负数、NaN、+Inf，且不修改任何状态 */
    if (!pid_is_finite(dt) || (dt <= 0.0f)) {
        return 0.0f;
    }

    /* 拒绝 NaN / Inf 输入，避免污染积分与微分先行状态 */
    if (!pid_is_finite(setpoint) || !pid_is_finite(measurement)) {
        return 0.0f;
    }

    /* 事务式更新：以下每一步都在提交前检查有限性，任一步溢出/非法
       即返回 0，且不修改任何内部状态（integral / prev_measurement /
       first 都保持原值）。 */
    error = setpoint - measurement;
    if (!pid_is_finite(error)) {
        return 0.0f; /* 误差溢出 */
    }

    increment = pid->ki * error * dt;
    if (!pid_is_finite(increment)) {
        return 0.0f; /* 积分增量溢出 */
    }

    new_integral = pid->integral + increment;
    if (!pid_is_finite(new_integral)) {
        return 0.0f; /* 积分候选值溢出 */
    }
    new_integral = clampf(new_integral, pid->integral_limit);

    if (pid->first) {
        d_term = 0.0f;
    } else {
        d_term = pid->kd * (pid->prev_measurement - measurement) / dt;
        if (!pid_is_finite(d_term)) {
            return 0.0f; /* 微分项溢出 */
        }
    }

    out = pid->kp * error + new_integral + d_term;
    if (!pid_is_finite(out)) {
        return 0.0f; /* 输出溢出 */
    }
    out = clampf(out, pid->output_limit);

    /* 全部检查通过，提交内部状态 */
    pid->integral = new_integral;
    pid->prev_measurement = measurement;
    pid->first = 0;

    return out;
}
