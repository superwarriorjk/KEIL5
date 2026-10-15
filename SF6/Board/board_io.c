#include "board_io.h"
#include "stm32f10x.h"

/* Preserve current wiring, pull-ups, SWD and PA0-PA9 open-drain writes. */
void GPIO_Init_All(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* PA0-PA9: open drain, initially released. Preserve upper CRH fields. */
    GPIOA->CRL &= ~(0xFFFFFFFF);
    GPIOA->CRL |= 0x77777777;
    GPIOA->CRH &= ~(0xFF);
    GPIOA->CRH |= 0x77;
    GPIOA->ODR |= 0x03FF;

    /* PB0-PB15 and PA10-PA12: active-low inputs with pull-ups. */
    GPIOB->CRL &= ~(0xFFFFFFFF);
    GPIOB->CRL |= 0x88888888;
    GPIOB->CRH &= ~(0xFFFFFFFF);
    GPIOB->CRH |= 0x88888888;
    GPIOB->ODR |= 0xFFFF;
    GPIOA->CRH &= ~((0xF << 8) | (0xF << 12) | (0xF << 16));
    GPIOA->CRH |= (0x8 << 8) | (0x8 << 12) | (0x8 << 16);
    GPIOA->ODR |= (1 << 10) | (1 << 11) | (1 << 12);
}

IN_STATE ReadInputs(void)
{
    IN_STATE state = 0;
    uint32_t pb = ~GPIOB->IDR;
    uint32_t pa = ~GPIOA->IDR;
    state |= (pb & 0xFFFF);
    state |= ((pa >> 10) & 0x7) << 16;
    return state;
}

void Output_Update(OUT_STATE out)
{
    GPIOA->ODR |= 0x03FF;
    GPIOA->ODR &= ~(out & 0x03FF);
}
