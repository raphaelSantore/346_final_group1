#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>

typedef enum {IDLE, HOMING, ACCEL, CRUISE, DECEL, FAULT} state_t;

extern volatile state_t state;
extern volatile int32_t cmd_steps;
extern volatile uint8_t ms_setting;

void Stepper_Init(void);
uint8_t Stepper_SetMicrostep(uint8_t n);
void Stepper_RunConstant(uint32_t rate_sps, int8_t direction);
void Stepper_StopNow(void);
void Stepper_Home(void);
void Stepper_SetHomed(void);
uint8_t Stepper_IsHomed(void);

#endif