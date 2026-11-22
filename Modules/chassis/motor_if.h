#ifndef MOTOR_IF_H
#define MOTOR_IF_H

#include <stdint.h>

typedef enum { MOTOR_FL = 0, MOTOR_FR, MOTOR_RL, MOTOR_RR, MOTOR_COUNT } MotorId;

typedef struct {
    void    (*set_speed)(MotorId id, int32_t steps_per_sec); /* 正负表示方向 */
    void    (*stop)(MotorId id);
    void    (*stop_all)(void);
    int32_t (*get_position)(MotorId id);                     /* 可为 NULL */
} MotorIf;

extern MotorIf g_motor;   /* 由 BSP/board 的 board_init() 填充，现在不要实现 */

#endif /* MOTOR_IF_H */
