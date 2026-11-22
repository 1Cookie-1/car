#ifndef CRC8_H
#define CRC8_H

#include <stdint.h>

/*
 * CRC-8 参数（协议确认后以协议为准）：
 *   - 多项式：0x07
 *   - 初值：0x00
 *   - 输入输出均不反射
 *   - 输出不异或（XOROUT = 0x00）
 */
#define CRC8_POLY   0x07u
#define CRC8_INIT   0x00u
#define CRC8_XOROUT 0x00u

uint8_t Crc8_Update(uint8_t crc, uint8_t byte);
uint8_t Crc8_Compute(const uint8_t *data, uint16_t len);

#endif /* CRC8_H */
