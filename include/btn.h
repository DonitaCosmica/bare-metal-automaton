#ifndef BTN_H
#define BTN_H

#include <stdint.h>
#include "util.h"

#define BTN_ENTRANCE_PORT 3 // P302 - D1
#define BTN_ENTRANCE_PIN 2
#define BTN_TICKET_PORT 1 // P105 - D3
#define BTN_TICKET_PIN 5
#define BTN_COIN5_PORT 1 // P106 - D4
#define BTN_COIN5_PIN 6
#define BTN_COIN10_PORT 1 // P107 - D5
#define BTN_COIN10_PIN 7
#define BTN_CANCEL_PORT 1 // P111 - D6
#define BTN_CANCEL_PIN 11
#define BTN_EXIT_PORT 3 // P303 - D9
#define BTN_EXIT_PIN 3
#define BTN_INV_COIN_PORT 1 // P102 - D13
#define BTN_INV_COIN_PIN 2

#define BTN_DEBOUNCE_MS 50u

uint32_t button_read(uint8_t, uint8_t);
void button_init(uint8_t, uint8_t, btn_state_t*);
uint32_t button_pressed(uint8_t, uint8_t, btn_state_t*);

#endif