#ifndef PARKING_H
#define PARKING_H

#include <stdint.h>
#include "util.h"

#define LOT_SIZE 10u
#define BASE_FEE 10u
#define FEE_PER_HOUR 5u
#define MAX_HOURS 10u
#define COIN_TIMEOUT_MS 10000u
#define EXIT_TIMEOUT_MS 15000u
#define NOTICE_MS 2500u

typedef enum {
  Q0 = 0,
  Q1 = 1,
  Q2 = 2,
  Q3 = 3,
  Q4 = 4,
  Q5 = 5,
  Q6 = 6,
  Q7 = 7,
  Q8 = 8,
  Q9 = 9
} parking_state_t;

typedef enum {
  TK_FREE = 0, // Casilla libre
  TK_ACTIVE = 1, // Vehiculo estacionado, sin pagar o pago perdido
  TK_PAID = 2 // pagado y autorizado para salir
} ticket_status_t;

typedef struct {
  uint32_t serial;
  uint32_t hours;
  uint32_t status;
} ticket_t;

typedef struct {
  const gate_cfg_t *entrance;
  const gate_cfg_t *exit_gate;
  const payment_cfg_t *pay;

  gate_state_t entrance_state;
  gate_state_t exit_state;
  payment_state_t pay_state;

  parking_state_t state;
  uint32_t entering;
  uint32_t full_shown;
  uint32_t notice;
  uint32_t notice_t0;

  uint32_t occupied;
  uint32_t next_serial;
  uint32_t rng;
  ticket_t tickets[LOT_SIZE];

  int32_t cur;
  uint32_t total;
  uint32_t paid;
  uint32_t change;
  uint32_t timer_start;
} parking_t;

void parking_init(parking_t*, const gate_cfg_t*, const gate_cfg_t*, const payment_cfg_t*);
void parking_update(parking_t*);

#endif