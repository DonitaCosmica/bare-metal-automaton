#include "timebase.h"

#define MSTPCRD (*(volatile uint32_t *)0x40047008)
#define MSTPD5_GPT32 (1u << 5);

#define GPT0_BASE 0x40078000u
#define GPT0_GTCR (*(volatile uint32_t *)(GPT0_BASE + 0x2C))
#define GPT0_GTCNT (*(volatile uint32_t *)(GPT0_BASE + 0x48))
#define GPT0_GTPR (*(volatile uint32_t *)(GPT0_BASE + 0x64))

#define GTCR_CST (1u << 0)
#define GTCR_TPCS(n) ((uint32_t)(n) << 24)

void timebase_init(void) {
  MSTPCRD &= ~MSTPD5_GPT32;

  GPT0_GTCR = GTCR_TPCS(TIME_TPCS);
  GPT0_GTCNT = 0u;
  GPT0_GTPR = 0xFFFFFFFFu;
  GPT0_GTCR |= GTCR_CST;
}

uint32_t timer_now(void) {
  return GPT0_GTCNT;
}

uint32_t timer_elapsed_ms(uint32_t start) {
  return(timer_now() - start) / TICKS_PER_MS;
}