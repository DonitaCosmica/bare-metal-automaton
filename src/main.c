#include <stdint.h>

#include "util.h"
#include "fc.h"
#include "led.h"
#include "btn.h"
#include "servo.h"
#include "timebase.h"
#include "ui.h"
#include "parking.h"

#define CALIBRATION_MODE 0

extern uint32_t _estack;
void Reset_Handler(void);
void Default_Handler(void);

__attribute__((section(".isr_vector"), used))
void (*const vectors[16])(void) = {
  (void (*)(void))&_estack,
  Reset_Handler,
  Default_Handler, Default_Handler, Default_Handler, Default_Handler,
  Default_Handler, 0, 0, 0, 0,
  Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler
};

#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08)
#define PWPR (*(volatile uint8_t *)0x40040D03)

static const gate_cfg_t ENTRANCE_GATE = {
  FC_ENTRANCE_PORT, FC_ENTRANCE_PIN, BTN_ENTRANCE_PORT, BTN_ENTRANCE_PIN,
  LED_ENTRANCE_PORT, LED_ENTRANCE_PIN, GTCCRA_OFF
};

static const gate_cfg_t EXIT_GATE = {
  FC_EXIT_PORT, FC_EXIT_PIN, BTN_EXIT_PORT, BTN_EXIT_PIN,
  LED_EXIT_PORT, LED_EXIT_PIN, GTCCRB_OFF
};

static const payment_cfg_t PAYMENT = {
  BTN_TICKET_PORT, BTN_TICKET_PIN,
  BTN_COIN5_PORT, BTN_COIN5_PIN,
  BTN_COIN10_PORT, BTN_COIN10_PIN,
  BTN_CANCEL_PORT, BTN_CANCEL_PIN,
  BTN_INV_COIN_PORT, BTN_INV_COIN_PIN,
  LED_TICKET_PORT, LED_TICKET_PIN
};

int main(void) {
  SCB_VTOR = 0x00004000;

  PWPR = 0x00;
  PWPR = 0x40;

  timebase_init();

#if CALIBRATION_MODE
  gpt6_pwm_init(5u, PCLKD_HZ / 1024u, (PCLKD_HZ / 1024u) / 2u, (PCLKD_HZ / 1024u) / 2u);
  while (1);
#else
  parking_t parking;

  gpt6_pwm_init(PWM_TPCS, PERIOD_TICKS, angle_to_ticks(0u), angle_to_ticks(0u));
  ui_init();
  parking_init(&parking, &ENTRANCE_GATE, &EXIT_GATE, &PAYMENT);

  while(1) {
    parking_update(&parking);
    delay(SHOW_DELAY);
  }
#endif

  return 0;
}

void Default_Handler(void) { while(1); }
void Reset_Handler(void) {
  main();
  while (1);
}