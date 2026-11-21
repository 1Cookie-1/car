#include "led.h"
#include "main.h"

/* 板载 LED，引脚暂时写死 PA5 */

#define LED_PORT  GPIOA
#define LED_PIN   GPIO_PIN_5

static void led_write(int on)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin   = LED_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    led_write(0);
}

void Led_On(void)     { led_write(1); }
void Led_Off(void)    { led_write(0); }
void Led_Toggle(void) { HAL_GPIO_TogglePin(LED_PORT, LED_PIN); }