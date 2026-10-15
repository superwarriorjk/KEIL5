#ifndef TEST_STM32F10X_H
#define TEST_STM32F10X_H
#include <stdint.h>
/* Host-only register model. Never included in the Keil target. */
typedef struct { uint32_t CRL, CRH, IDR, ODR; } TestGPIO;
typedef struct { uint32_t APB2ENR; } TestRCC;
typedef struct { uint32_t MAPR; } TestAFIO;
typedef struct { uint32_t LOAD, VAL, CTRL; } TestSysTick;
extern TestGPIO test_gpio_a, test_gpio_b;
extern TestRCC test_rcc;
extern TestAFIO test_afio;
extern TestSysTick test_systick;
#define GPIOA (&test_gpio_a)
#define GPIOB (&test_gpio_b)
#define RCC (&test_rcc)
#define AFIO (&test_afio)
#define SysTick (&test_systick)
#define RCC_APB2ENR_AFIOEN (1U << 0)
#define RCC_APB2ENR_IOPAEN (1U << 2)
#define RCC_APB2ENR_IOPBEN (1U << 3)
#define AFIO_MAPR_SWJ_CFG_1 (1U << 25)
#define SysTick_CTRL_CLKSOURCE_Msk (1U << 2)
#define SysTick_CTRL_TICKINT_Msk (1U << 1)
#define SysTick_CTRL_ENABLE_Msk (1U << 0)
#endif
