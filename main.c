#include "stm32l476xx.h"
#include "SysClock.h"
#include "config.h"
#include "stepper.h"

void LED_D8_Init(void);
void A2_EXTI_Init(void);

int main(void) {
    System_Clock_Init();

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    LED_D8_Init();
    A2_EXTI_Init();

    Stepper_Init();
    Stepper_RunConstant(TEST_RATE_SPS, 1);

    while (1) {
    }
}

void LED_D8_Init(void) {
    GPIOA->MODER  &= ~(3UL << (2U * LED_D8_PIN));
    GPIOA->MODER  |=  (1UL << (2U * LED_D8_PIN));
    GPIOA->OTYPER &= ~(1UL << LED_D8_PIN);
    GPIOA->PUPDR  &= ~(3UL << (2U * LED_D8_PIN));
    GPIOA->ODR    &= ~(1UL << LED_D8_PIN);
}

void A2_EXTI_Init(void) {
    GPIOA->MODER &= ~(3UL << (2U * BUTTON_A2_PIN));
    GPIOA->PUPDR &= ~(3UL << (2U * BUTTON_A2_PIN));
    GPIOA->PUPDR |=  (1UL << (2U * BUTTON_A2_PIN));

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI4;

    EXTI->IMR1  |=  EXTI_IMR1_IM4;
    EXTI->RTSR1 &= ~EXTI_RTSR1_RT4;
    EXTI->FTSR1 |=  EXTI_FTSR1_FT4;
    EXTI->PR1    =  EXTI_PR1_PIF4;

    NVIC_SetPriority(EXTI4_IRQn, 0U);
    NVIC_EnableIRQ(EXTI4_IRQn);
}

void EXTI4_IRQHandler(void) {
    if ((EXTI->PR1 & EXTI_PR1_PIF4) != 0U) {
        EXTI->PR1 = EXTI_PR1_PIF4;
        Stepper_StopNow();
        GPIOA->ODR ^= (1UL << LED_D8_PIN);
    }
}