#include "parking.h"
#include "gate.h"
#include "btn.h"
#include "led.h"
#include "ui.h"
#include "timebase.h"

static uint32_t rng_next(uint32_t *s) {
  uint32_t x = *s;
  x ^= x << 13;
  x ^= x >>17;
  x ^= x << 5;
  *s = x;
  return x;
}

static uint32_t random_hours(parking_t *parking) {
  parking->rng ^= timer_now();
  if(parking->rng == 0u) parking->rng = 2463534242u;
  return (rng_next(&parking->rng) >> 8) % (MAX_HOURS + 1u);
}

static void ticket_led(const parking_t *parking, uint32_t on) {
  led_write(parking->pay->led_port, parking->pay->led_pin, on);
}

static void go(parking_t *parking, parking_state_t next) {
  parking->state = next;
  parking->entering = 1u;
  parking->notice = 0u;
}

static void redraw(parking_t *parking) {
  parking->entering = 1u;
}

static void start_notice(parking_t *parking) {
  parking->entering = 0u;
  parking->notice = 1u;
  parking->notice_t0 = timer_now();
}

static uint32_t lot_full(const parking_t *parking) {
  return parking->occupied >= LOT_SIZE;
}

static parking_state_t idle_state(const parking_t *parking) {
  return parking->occupied > 0u ? Q4 : Q0;
}

static uint32_t try_vehicle(parking_t *parking, uint32_t vehicle_present) {
  if(!vehicle_present) {
    if(parking->full_shown) {
      parking->full_shown = 0u;
      redraw(parking);
    }

    return 0u;
  }

  if(lot_full(parking)) {
    if(!parking->full_shown) {
      parking->full_shown = 1u;
      ui_show_message("Estacionamiento\nlleno");
    }

    return 0u;
  }

  return 1u;
}

static int32_t ticket_add(parking_t *parking, uint32_t serial, uint32_t hours) {
  for(uint32_t i = 0; i < LOT_SIZE; ++i) {
    if(parking->tickets[i].status == TK_FREE) {
      parking->tickets[i].serial = serial;
      parking->tickets[i].hours = hours;
      parking->tickets[i].status = TK_ACTIVE;
      return (int32_t)i;
    }
  }

  return -1;
}

static int32_t ticket_oldest_active(const parking_t *parking) {
  int32_t best = -1;
  for(uint32_t i = 0; i < LOT_SIZE; ++i) {
    if(parking->tickets[i].status != TK_ACTIVE) continue;
    if(best < 0 || parking->tickets[i].serial < parking->tickets[best].serial) best = (int32_t)i;
  }

  return best;
}

static uint32_t fee_for(uint32_t hours) {
  return BASE_FEE + FEE_PER_HOUR * hours;
}

static void abort_payment(parking_t *parking, const char *reason) {
  uint32_t refund = parking->paid;
  parking->paid = 0u;

  go(parking, Q4);
  ui_show_refund(reason, refund);
  ticket_led(parking, 1u);
  start_notice(parking);
}

static void accept_coin(parking_t *parking, uint32_t coin) {
  parking->paid += coin;
  parking->timer_start = timer_now();

  if(parking->paid >= parking->total) go(parking, Q7);
  else if(parking->state == Q6) redraw(parking);
  else go(parking, Q6);
}

static void exit_timeout(parking_t *parking, ticket_t *ticket) {
  ticket->status = TK_ACTIVE; // EL boleto pasa a ser no autorizado para salir
  ticket->hours = random_hours(parking); // Nuevo tiempo aleatorio
  parking->paid = 0u;
  ticket_led(parking, 0u);

  go(parking, Q4);
  ui_show_message("Tiempo agotado\nPague de nuevo");
  start_notice(parking);
}

void parking_init(parking_t *parking, const gate_cfg_t *entrance, const gate_cfg_t *exit_gate, const payment_cfg_t *pay) {
  parking->entrance = entrance;
  parking->exit_gate = exit_gate;
  parking->pay = pay;

  gate_init(entrance, &parking->entrance_state);
  gate_init(exit_gate, &parking->exit_state);

  button_init(pay->btn_tk_port, pay->btn_tk_pin, &parking->pay_state.ticket);
  button_init(pay->btn_c5_port, pay->btn_c5_pin, &parking->pay_state.coin5);
  button_init(pay->btn_c10_port, pay->btn_c10_pin, &parking->pay_state.coin10);
  button_init(pay->btn_cancel_port, pay->btn_cancel_pin, &parking->pay_state.cancel);
  ticket_led(parking, 0u);

  parking->state = Q0;
  parking->entering = 1u;
  parking->full_shown = 0u;
  parking->notice = 0u;
  parking->notice_t0 = 0u;

  parking->occupied = 0u;
  parking->next_serial = 1u;
  parking->rng = 2463534242u;
  for(uint32_t i = 0; i < LOT_SIZE; ++i) {
    parking->tickets[i].serial = 0u;
    parking->tickets[i].hours = 0u;
    parking->tickets[i].status = TK_FREE;
  }

  parking->cur = -1;
  parking->total = 0u;
  parking->paid = 0u;
  parking->change = 0u;
  parking->timer_start = 0u;
}

void parking_update(parking_t *parking) {
  const payment_cfg_t *pay = parking->pay;

  uint32_t vehicle_entrance = gate_sense(parking->entrance, &parking->entrance_state);
  uint32_t vehicle_exit = gate_sense(parking->exit_gate, &parking->exit_state);
  uint32_t ev_enter = button_pressed(parking->entrance->btn_port, parking->entrance->btn_pin, &parking->entrance_state.btn);
  uint32_t ev_ticket = button_pressed(pay->btn_tk_port, pay->btn_tk_pin, &parking->pay_state.ticket);
  uint32_t ev_c5 = button_pressed(pay->btn_c5_port, pay->btn_c5_pin, &parking->pay_state.coin5);
  uint32_t ev_c10 = button_pressed(pay->btn_c10_port, pay->btn_c10_pin, &parking->pay_state.coin10);
  uint32_t ev_cancel = button_pressed(pay->btn_cancel_port, pay->btn_cancel_pin, &parking->pay_state.cancel);
  uint32_t ev_exit = button_pressed(parking->exit_gate->btn_port, parking->exit_gate->btn_pin, &parking->exit_state.btn);
  uint32_t ev_inv_coin = button_pressed(pay->btn_inv_coin_port, pay->btn_inv_coin_pin, &parking->pay_state.invalid_coin);
  uint32_t coin = ev_c10 ? 10u : (ev_c5 ? 5u : 0u);

  if(parking->notice && timer_elapsed_ms(parking->notice_t0) >= NOTICE_MS) {
    parking->notice = 0u;
    ticket_led(parking, 0u);
    redraw(parking);
  }

  switch(parking->state) {

    /* Q0: Entrada libre */
    case Q0: {
      if(parking->entering) {
        parking->entering = 0u;
        ui_show_message("Q0: Esperando\nUsuario");
      }

      if(try_vehicle(parking, vehicle_entrance)) go(parking, Q1);
      break;
    }

    /* Q1: vehiculo detectado, espera que presione el boton */
    case Q1: {
      if(parking->entering) {
        parking->entering = 0u;
        ui_show_message("Q1: Bienvenido\nPresione boton");
      }

      if(ev_enter) go(parking, Q2);
      else if(!vehicle_entrance) go(parking, idle_state(parking)); // Se fue sin sacar boleto 
      break;
    }

    /* Q2: Generar boleto y lo registra */
    case Q2: {
      ui_show_message("Q2: Generando\nboleto...");

      uint32_t hours = random_hours(parking);
      uint32_t serial = parking->next_serial++;

      if(ticket_add(parking, serial, hours) >= 0) go(parking, Q3);
      else go(parking, idle_state(parking));   /* no debería pasar: el cupo se revisó en q0/q3 */
      break;
    }

    /* Q3: Barrera de entrada abierta */
    case Q3: {
      if(parking->entering) {
        parking->entering = 0u;
        gate_open(parking->entrance, &parking->entrance_state);
        ticket_led(parking, 1u);   /* boleto disponible */
        ui_show_message("Q3: Tome su boleto\ny avance");
      }

      if(!vehicle_entrance) { // El vehiculo ya paso (tomo el boleto y cruzo)
        gate_close(parking->entrance, &parking->entrance_state);
        ticket_led(parking, 0u);
        parking->occupied++;
        go(parking, Q4);
      }

      break;
    }

    /* Q4: vehiculo estacionado */
    case Q4: {
      if(parking->entering) {
        parking->entering = 0u;
        ticket_led(parking, 0u);
        ui_show_parked(parking->occupied, LOT_SIZE);
      }

      if(ev_ticket) {
        int32_t idx = ticket_oldest_active(parking);
        if(idx >= 0) {
          parking->cur = idx;
          go(parking, Q5);
          break;
        }
      }

      if(try_vehicle(parking, vehicle_entrance)) go(parking, Q1); // Otro vehículo llega a la entrada
      break;
    }

    /* Q5: El usuario introduce el boleto en la caja */
    case Q5: {
      const ticket_t *t = &parking->tickets[parking->cur];

      if(parking->entering) {
        parking->entering = 0u;
        ticket_led(parking, 0u);
        parking->total = fee_for(t->hours);
        parking->paid = 0u;
        parking->timer_start = timer_now(); // Empieza el timer entre monedas (10 s)
        ui_show_ticket_info(t->serial, t->hours, parking->total);
      }

      if(ev_cancel) {
        abort_payment(parking, "Cancelado");
      } else if(ev_inv_coin) {
        abort_payment(parking, "Moneda Invalida");
      } else if(timer_elapsed_ms(parking->timer_start) >= COIN_TIMEOUT_MS) {
        abort_payment(parking, "Tiempo agotado");
      } else if(coin) {
        accept_coin(parking, coin);
      }

      break;
    }

    /* Q6: El usuario introduce monedas una por una */
    case Q6: {
      if(parking->entering) {
        parking->entering = 0u;
        ui_show_money_paid(parking->total, parking->paid);
      }

      if(ev_cancel) {
        abort_payment(parking, "Cancelado");
      } else if(ev_inv_coin) {
        abort_payment(parking, "Moneda Invalida");
      } else if(timer_elapsed_ms(parking->timer_start) >= COIN_TIMEOUT_MS) {
        abort_payment(parking, "Tiempo agotado");
      } else if(coin) {
        accept_coin(parking, coin);
      }
      break;
    }

    /* Q7: Boleto pagado */
    case Q7: {
      ticket_t *ticket = &parking->tickets[parking->cur];

      if(parking->entering) {
        parking->entering = 0u;
        parking->change = parking->paid - parking->total;
        ticket->status = TK_PAID; // Salida autorizada
        ticket_led(parking, 1u); // Devuelve el boleto
        ui_show_paid(parking->paid, parking->change);
        parking->timer_start = timer_now(); // Empieza el timer de salida (15 s)
      }

      if(timer_elapsed_ms(parking->timer_start) >= EXIT_TIMEOUT_MS) exit_timeout(parking, ticket);
      if(vehicle_exit) go(parking, Q8);
      break;
    }

    /* Q8: Vehiculo detectado en la salida */
    case Q8: {
      ticket_t *ticket = &parking->tickets[parking->cur];

      if(parking->entering) {
        parking->entering = 0u;
        ui_show_message("Q8: Presione boton\npara salir");
      }

      if(ev_exit && ticket->status == TK_PAID) go(parking, Q9);
      else if(timer_elapsed_ms(parking->timer_start) >= EXIT_TIMEOUT_MS) exit_timeout(parking, ticket);
      else if(!vehicle_exit) go(parking, Q7); // El vehiculo volvio al estacionamiento
      break;
    }

    /* Q9: Boleto aceptada. Salida autorizada */
    case Q9: {
      ticket_t *ticket = &parking->tickets[parking->cur];

      if(parking->entering) {
        parking->entering = 0u;
        gate_open(parking->exit_gate, &parking->exit_state);
        ticket_led(parking, 1u);
        ui_show_message("Q9: Gracias por su visita");
      }

      if(!vehicle_exit) { // El vehiculo salio
        gate_close(parking->exit_gate, &parking->exit_state);
        ticket_led(parking, 0u);
        parking->occupied--;
        ticket->status = TK_FREE;
        go(parking, idle_state(parking));
      }

      break;
    }

    default:
      go(parking, idle_state(parking));
      break;
  }
}