#include "util.h"

void delay(volatile uint32_t n) {
  while (n--) __asm__("nop");
}