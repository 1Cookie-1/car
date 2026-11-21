#ifndef APP_LED_H
#define APP_LED_H

/* 板载 LED 控制。引脚暂时写死 PA5，硬件到了再改 */

void Led_Init(void);
void Led_On(void);
void Led_Off(void);
void Led_Toggle(void);

#endif /* APP_LED_H */