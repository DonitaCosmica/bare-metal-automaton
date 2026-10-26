#ifndef SERVO_H
#define SERVO_H

#define PCLKD_HZ 48000000u

#define PWM_TPCS 2u
#define PWM_DIV 16u
#define SERVO_HZ 50u

#define TIMER_HZ (PCLKD_HZ / PWM_DIV)
#define PERIOD_TICKS (TIMER_HZ / SERVO_HZ)

#include <stdint.h>

void gpt0_pwm_init(uint32_t, uint32_t, uint32_t);
uint32_t angle_to_ticks(uint32_t);
void servo_set_angle(uint32_t);

#endif