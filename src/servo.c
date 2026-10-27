#include "servo.h"
#include "util.h"

#define SRV_PSEL 3u

#define MSTPCRD (*(volatile uint32_t *)0x40047008)
#define MSTPD6_GPT16 (1u << 6)

#define GPT6_BASE 0x40078600u
#define GPT6_GTCR (*(volatile uint32_t *)(GPT6_BASE + 0x2C))
#define GPT6_GTIOR (*(volatile uint32_t *)(GPT6_BASE + 0x34))
#define GPT6_GTCNT (*(volatile uint32_t *)(GPT6_BASE + 0x48))
#define GPT6_GTCCRA (*(volatile uint32_t *)(GPT6_BASE + 0x4C))
#define GPT6_GTCCRB (*(volatile uint32_t *)(GPT6_BASE + 0x50))
#define GPT6_GTPR (*(volatile uint32_t *)(GPT6_BASE + 0x64))

#define GTCR_CST (1u << 0)
#define GTCR_TPCS(n) ((uint32_t)(n) << 24)

#define GTIOR_GTIOA(v) ((uint32_t)(v))
#define GTIOR_OAE (1u << 8)
#define GTIOR_GTIOB(v) ((uint32_t)(v) << 16)
#define GTIOR_OBE (1u << 24)

#define SRV_MIN_US 544u
#define SRV_MAX_US 2400u

void gpt6_pwm_init(uint32_t tpcs, uint32_t period_ticks, uint32_t duty_a, uint32_t duty_b) {
  MSTPCRD &= ~MSTPD6_GPT16;

  GPT6_GTCR = GTCR_TPCS(tpcs);
  GPT6_GTCNT = 0;
  GPT6_GTPR = period_ticks - 1u;
  GPT6_GTCCRA = duty_a;
  GPT6_GTCCRB = duty_b;
  GPT6_GTIOR = GTIOR_GTIOA(25) | GTIOR_OAE | GTIOR_GTIOB(25) | GTIOR_OBE; 
  
  PFS(SRV_ENTRANCE_PORT, SRV_ENTRANCE_PIN) = 0;
  PFS(SRV_ENTRANCE_PORT, SRV_ENTRANCE_PIN) = PFS_PSEL(SRV_PSEL) | PFS_PMR;
  PFS(SRV_EXIT_PORT, SRV_EXIT_PIN) = 0;
  PFS(SRV_EXIT_PORT, SRV_EXIT_PIN) = PFS_PSEL(SRV_PSEL) | PFS_PMR;

  GPT6_GTCR |= GTCR_CST;
}

uint32_t angle_to_ticks(uint32_t angle) {
  uint32_t us = SRV_MIN_US + ((SRV_MAX_US- SRV_MIN_US) * angle) / 180u;
  return (us * (TIMER_HZ / 1000u)) / 1000u;
}

void servo_set_angle(const gate_cfg_t *u, uint32_t angle) {
  *(volatile uint32_t *)(GPT6_BASE + u->ccr_off) = angle_to_ticks(angle);
}