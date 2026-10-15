#include "frame_clock.h"
#include "../Core/controller_config.h"
#include "stm32f10x.h"
volatile uint8_t tick_flag = 0;
/* Retain the current 1,200,000 HCLK cycles per frame (72 MHz / 60). */
void SysTick_Init(void)
{
    SysTick->LOAD = 1200000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}
void SysTick_Handler(void) { tick_flag = 1; }
