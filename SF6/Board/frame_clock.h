#ifndef SF6_FRAME_CLOCK_H
#define SF6_FRAME_CLOCK_H
#include <stdint.h>
extern volatile uint8_t tick_flag;
void SysTick_Init(void);
void SysTick_Handler(void);
#endif
