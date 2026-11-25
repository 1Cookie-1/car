#ifndef MOTOR_IF_H
#define MOTOR_IF_H

#include <stdint.h>

/*
 * 电机抽象接口（Modules 层，不依赖 HAL 或具体硬件）。
 *
 * MotorId 合法范围：MOTOR_FL .. MOTOR_RR；MOTOR_COUNT 仅用于计数，
 * 不是合法电机 ID。调用方必须保证传入的 id < MOTOR_COUNT。
 *
 * 速度单位：set_speed 的 steps_per_sec 是步进脉冲频率，单位 steps/s，
 * 符号表示方向（正负由具体驱动约定）。本接口不约定 steps 与毫米、
 * 转速之间的换算系数；换算由上层结合轮径、细分等实际参数完成，
 * 硬件未确认前不得虚构。
 *
 * 语义：
 *   - set_speed(id, steps_per_sec)：设定单台电机速度；
 *   - stop(id)：停止单台电机；
 *   - stop_all()：停止全部电机；
 *   - get_position(id)：返回累计位置（步数）。若实现不支持位置反馈，
 *     该指针可为 NULL；0 是合法位置值，不能作为错误标志，调用前必须
 *     检查 get_position != NULL，并且它不提供错误返回通道。
 *
 * 调用前提：
 *   g_motor 中的函数指针由 BSP/board 的 Board_Init() 在底层驱动初始化
 *   完成后绑定。绑定完成前函数指针可能为 NULL（或未绑定），上层不得
 *   调用；调用方应自行保证绑定完成后才通过 g_motor 访问。
 *
 * g_motor 当前只有声明、没有定义：定义由未来 BSP/board 提供。因此在
 * BSP/board 落地前，任何引用 g_motor 的代码都无法链接；目前生产代码中
 * 也没有任何地方引用 g_motor。
 */
typedef enum { MOTOR_FL = 0, MOTOR_FR, MOTOR_RL, MOTOR_RR, MOTOR_COUNT } MotorId;

typedef struct {
    void    (*set_speed)(MotorId id, int32_t steps_per_sec); /* 正负表示方向 */
    void    (*stop)(MotorId id);
    void    (*stop_all)(void);
    int32_t (*get_position)(MotorId id);                     /* 可为 NULL */
} MotorIf;

extern MotorIf g_motor;   /* 由 BSP/board 的 Board_Init() 填充，现在不要实现 */

#endif /* MOTOR_IF_H */
