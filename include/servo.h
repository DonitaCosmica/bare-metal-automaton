#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "util.h"

#define SRV_ENTRANCE_PORT 4 // P411 - D11
#define SRV_ENTRANCE_PIN 11
#define SRV_EXIT_PORT 4 // P410 - D12
#define SRV_EXIT_PIN 10

#define GTCCRA_OFF 0x4Cu
#define GTCCRB_OFF 0x50u

#define PCLKD_HZ 48000000u

#define PWM_TPCS 2u
#define PWM_DIV 16u
#define SERVO_HZ 50u

#define TIMER_HZ (PCLKD_HZ / PWM_DIV)
#define PERIOD_TICKS (TIMER_HZ / SERVO_HZ)

void gpt6_pwm_init(uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t angle_to_ticks(uint32_t);
void servo_set_angle(const gate_cfg_t*, uint32_t);

#endif