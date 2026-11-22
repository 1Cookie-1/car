#include "pid.h"

static float clampf(float v, float limit)
{
    if (v > limit) {
        return limit;
    }
    if (v < -limit) {
        return -limit;
    }
    return v;
}

void Pid_Init(Pid *pid, float kp, float ki, float kd,
              float integral_limit, float output_limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
    pid->prev_measurement = 0.0f;
    pid->first = 1;
}

void Pid_Reset(Pid *pid)
{
    pid->integral = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->first = 1;
}

float Pid_Update(Pid *pid, float setpoint, float measurement, float dt)
{
    float error = setpoint - measurement;
    float out;
    float d_term = 0.0f;

    if (dt <= 0.0f) {
        return 0.0f;
    }

    pid->integral += pid->ki * error * dt;
    pid->integral = clampf(pid->integral, pid->integral_limit);

    if (!pid->first) {
        d_term = pid->kd * (pid->prev_measurement - measurement) / dt;
    }

    pid->prev_measurement = measurement;
    pid->first = 0;

    out = pid->kp * error + pid->integral + d_term;
    out = clampf(out, pid->output_limit);

    return out;
}
