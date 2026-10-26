#include "servo.h"
#include "util.h"

#define SRV_PORT 1
#define SRV_PIN 6

#define SRV_PSEL 3u

#define MSTPCRD (*(volatile uint32_t *)0x40047008)
#define MSTPD5_GPT32 (1u << 5)

#define GPT0_BASE 0x40078000u
#define GPT0_GTCR (*(volatile uint32_t *)(GPT0_BASE + 0x2C))
#define GPT0_GTIOR (*(volatile uint32_t *)(GPT0_BASE + 0x34))
#define GPT0_GTCNT (*(volatile uint32_t *)(GPT0_BASE + 0x48))
#define GPT0_GTCCRB (*(volatile uint32_t *)(GPT0_BASE + 0x50))
#define GPT0_GTPR (*(volatile uint32_t *)(GPT0_BASE + 0x64))

#define GTCR_CST (1u << 0)
#define GTCR_TPCS(n) ((uint32_t)(n) << 24)

#define GTIOR_GTIOB(v) ((uint32_t)(v) << 16)
#define GTIOR_OBE (1u << 24)

#define SERVO_MIN_US 544u
#define SERVO_MAX_US 2400u

void gpt0_pwm_init(uint32_t tpcs, uint32_t period_ticks, uint32_t duty_ticks) {
  MSTPCRD &= ~MSTPD5_GPT32;

  GPT0_GTCR = GTCR_TPCS(tpcs);
  GPT0_GTCNT = 0;
  GPT0_GTPR = period_ticks - 1u;
  GPT0_GTCCRB = duty_ticks;
  GPT0_GTIOR = GTIOR_GTIOB(25) | GTIOR_OBE;

  PFS(SRV_PORT, SRV_PIN) = 0;
  PFS(SRV_PORT, SRV_PIN) = PFS_PSEL(SRV_PSEL) | PFS_PMR;

  GPT0_GTCR  |= GTCR_CST;
}

uint32_t angle_to_ticks(uint32_t angle) {
  uint32_t us = SERVO_MIN_US + ((SERVO_MAX_US - SERVO_MIN_US) * angle) / 180u;
  return (us * (TIMER_HZ / 1000u)) / 1000u;
}

void servo_set_angle(uint32_t angle) {
  GPT0_GTCCRB = angle_to_ticks(angle);
}