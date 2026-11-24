#include "stm32l476xx.h"
#include "config.h"
#include "stepper.h"

volatile state_t state = IDLE;
volatile int32_t cmd_steps = 0;
volatile uint8_t ms_setting = 1;

static volatile int8_t dir = 1;
static volatile uint8_t homed = 0;
static volatile int32_t min_steps = 0;
static volatile int32_t max_steps = 0;

static void gpio_out(uint32_t pin) {
    GPIOB->MODER  &= ~(3UL << (2U * pin));
    GPIOB->MODER  |=  (1UL << (2U * pin));
    GPIOB->OTYPER &= ~(1UL << pin);
    GPIOB->PUPDR  &= ~(3UL << (2U * pin));
}

static void start_motion(uint32_t rate_sps, int8_t direction, state_t st) {
    volatile uint32_t i;

    if (rate_sps < MIN_RATE_SPS) {
        rate_sps = MIN_RATE_SPS;
    }

    dir = (direction >= 0) ? 1 : -1;

    if (dir > 0) {
        GPIOB->BSRR = (1UL << DIR_PIN);
    } else {
        GPIOB->BSRR = (1UL << (DIR_PIN + 16U));
    }

    GPIOB->BSRR = (1UL << (EN_PIN + 16U));

    for (i = 0; i < 200U; i++) {
    }

    TIM1->ARR = (STEP_TIM_HZ / rate_sps) - 1U;
    TIM1->CNT = 0;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR  = 0;
    state = st;
    TIM1->CR1 |= TIM_CR1_CEN;
}

void Stepper_Init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    GPIOB->BSRR = (1UL << EN_PIN);
    gpio_out(EN_PIN);
    gpio_out(DIR_PIN);
    gpio_out(MS1_PIN);
    gpio_out(MS2_PIN);
    gpio_out(MS3_PIN);

    GPIOA->MODER   &= ~(3UL << (2U * STEP_PIN));
    GPIOA->MODER   |=  (2UL << (2U * STEP_PIN));
    GPIOA->AFR[1]  &= ~(0xFUL << (4U * (STEP_PIN - 8U)));
    GPIOA->AFR[1]  |=  (1UL << (4U * (STEP_PIN - 8U)));
    GPIOA->OSPEEDR |=  (3UL << (2U * STEP_PIN));

    TIM1->CR1   = 0;
    TIM1->PSC   = (SYSCLK_HZ / STEP_TIM_HZ) - 1U;
    TIM1->ARR   = (STEP_TIM_HZ / MIN_RATE_SPS) - 1U;
    TIM1->CCR1  = STEP_PULSE_TICKS;
    TIM1->CCMR1 = (6UL << 4) | TIM_CCMR1_OC1PE;
    TIM1->CCER  = TIM_CCER_CC1E;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1  |= TIM_CR1_ARPE;
    TIM1->EGR   = TIM_EGR_UG;
    TIM1->SR    = 0;
    TIM1->DIER |= TIM_DIER_UIE;

    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1U);
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

    Stepper_SetMicrostep(DEFAULT_MSTEP);
}

uint8_t Stepper_SetMicrostep(uint8_t n) {
    uint32_t b;

    if (state != IDLE) {
        return 0;
    }

#if DRIVER_DRV8825
    switch (n) {
        case 1:  b = 0; break;
        case 2:  b = 1; break;
        case 4:  b = 2; break;
        case 8:  b = 3; break;
        case 16: b = 4; break;
        default: return 0;
    }
#else
    switch (n) {
        case 1:  b = 0; break;
        case 2:  b = 1; break;
        case 4:  b = 2; break;
        case 8:  b = 3; break;
        case 16: b = 7; break;
        default: return 0;
    }
#endif

    GPIOB->BSRR = ((1UL << MS1_PIN) | (1UL << MS2_PIN) | (1UL << MS3_PIN)) << 16;
    if (b & 1U) GPIOB->BSRR = (1UL << MS1_PIN);
    if (b & 2U) GPIOB->BSRR = (1UL << MS2_PIN);
    if (b & 4U) GPIOB->BSRR = (1UL << MS3_PIN);

    ms_setting = n;
    homed = 0;
    return 1;
}

void Stepper_RunConstant(uint32_t rate_sps, int8_t direction) {
    start_motion(rate_sps, direction, CRUISE);
}

void Stepper_StopNow(void) {
    TIM1->CR1 &= ~TIM_CR1_CEN;
    GPIOB->BSRR = (1UL << EN_PIN);
    state = IDLE;
}

void Stepper_Home(void) {
    homed = 0;
    start_motion(HOME_RATE_SPS, -1, HOMING);
}

void Stepper_SetHomed(void) {
    int32_t spmm = (int32_t)(FULL_STEPS_PER_MM * ms_setting);

    cmd_steps = 0;
    min_steps = (int32_t)SOFT_MARGIN_MM * spmm;
    max_steps = (int32_t)(TRAVEL_MM - SOFT_MARGIN_MM) * spmm;
    homed = 1;
    state = IDLE;
}

uint8_t Stepper_IsHomed(void) {
    return homed;
}

void TIM1_UP_TIM16_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR = ~TIM_SR_UIF;
        cmd_steps += dir;

        if (homed && (state != HOMING)) {
            if (((dir > 0) && (cmd_steps >= max_steps)) ||
                ((dir < 0) && (cmd_steps <= min_steps))) {
                Stepper_StopNow();
            }
        }
    }
}