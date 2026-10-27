#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "util.h"

#define LED_ENTRANCE_PORT 1 // P104 - D2
#define LED_ENTRANCE_PIN 4
#define LED_TICKET_PORT 1 // P112 - D7
#define LED_TICKET_PIN 12
#define LED_EXIT_PORT 1 // P103 - D10
#define LED_EXIT_PIN 3

void led_write(uint8_t, uint8_t, uint32_t);
void led_set(const gate_cfg_t*, uint32_t);
void led_blink(uint32_t);

#endif