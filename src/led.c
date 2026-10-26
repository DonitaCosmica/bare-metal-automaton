#include "led.h"
#include "util.h"

#define LED_PORT 1
#define LED_PIN 4

void led_set(uint32_t on) {
  PFS(LED_PORT, LED_PIN) = PFS_PDR | (on ? PFS_PODR : 0u);
}

void led_blink(uint32_t times) {
  for(uint32_t i = 0; i < times; ++i) {
    led_set(1); delay(800000);
    led_set(0); delay(800000);
  }
}