#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>
#include "servo.h"

#define TIME_TPCS 2u
#define TIME_DIV 16u
#define TICKS_PER_MS ((PCLKD_HZ / TIME_DIV) / 1000u)

void timebase_init(void);
uint32_t timer_now(void);
uint32_t timer_elapsed_ms(uint32_t);

#endif