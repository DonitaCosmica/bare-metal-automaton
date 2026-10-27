#include "led.h"

void led_write(uint8_t port, uint8_t pin, uint32_t on) {
  PFS(port, pin) = PFS_PDR | (on ? PFS_PODR : 0u);
}

void led_set(const gate_cfg_t *unit, uint32_t on) {
  led_write(unit->led_port, unit->led_pin, on);
}

void led_blink(uint32_t times) {
  for(uint32_t i = 0; i < times; ++i) {
    PFS(LED_TICKET_PORT, LED_TICKET_PIN) = PFS_PDR | PFS_PODR;
    delay(800000);
    PFS(LED_TICKET_PORT, LED_TICKET_PIN) = PFS_PDR;
    delay(800000);
  }
}