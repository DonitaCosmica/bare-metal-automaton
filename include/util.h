#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define PFS_BASE 0x40040800u
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

#define PFS_PODR (1u << 0)
#define PFS_PIDR (1u << 1)
#define PFS_PDR (1u << 2)
#define PFS_PCR (1u << 4)
#define PFS_PMR (1u << 16)
#define PFS_PSEL(n) ((uint32_t)(n) << 24)

typedef struct {
  uint32_t prev;
  uint32_t t_last;
  uint32_t used;
} btn_state_t;

typedef struct {
  uint8_t fc_port, fc_pin;
  uint8_t btn_port, btn_pin;
  uint8_t led_port, led_pin;
  uint8_t ccr_off;
} gate_cfg_t;

typedef struct {
  uint8_t btn_tk_port, btn_tk_pin;
  uint8_t btn_c5_port, btn_c5_pin;
  uint8_t btn_c10_port, btn_c10_pin;
  uint8_t btn_cancel_port, btn_cancel_pin;
  uint8_t btn_inv_coin_port, btn_inv_coin_pin;
  uint8_t led_port, led_pin;
} payment_cfg_t;

typedef struct {
  uint32_t detected;
  uint32_t pending;
  uint32_t t_change;
  uint32_t angle;
  btn_state_t btn;
} gate_state_t;

typedef struct {
  btn_state_t ticket, coin5, coin10, cancel, invalid_coin;
} payment_state_t;

void delay(volatile uint32_t);

#endif