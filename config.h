#ifndef CONFIG_H
#define CONFIG_H

#define DRIVER_DRV8825 1

#define SYSCLK_HZ 80000000UL
#define STEP_TIM_HZ 1000000UL
#define STEP_PULSE_TICKS 5U

#define STEP_PIN 8U
#define DIR_PIN 0U
#define EN_PIN 1U
#define MS1_PIN 13U
#define MS2_PIN 14U
#define MS3_PIN 15U

#define HOME_PIN 0U
#define FAR_PIN 1U

#define LED_D8_PIN 9U
#define BUTTON_A2_PIN 4U

#define FULL_STEPS_PER_MM 5U
#define TRAVEL_MM 300U
#define SOFT_MARGIN_MM 5U

#define MIN_RATE_SPS 100U
#define HOME_RATE_SPS 800U
#define TEST_RATE_SPS 800U
#define DEFAULT_MSTEP 16U

#endif