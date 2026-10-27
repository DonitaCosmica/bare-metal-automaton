#include "gate.h"
#include "fc.h"
#include "btn.h"
#include "led.h"
#include "servo.h"
#include "timebase.h"

void gate_init(const gate_cfg_t *gate, gate_state_t *state) {
  PFS(gate->fc_port, gate->fc_pin) = PFS_PCR;
  PFS(gate->led_port, gate->led_pin) = PFS_PDR;
  delay(1000);

  state->detected = object_detected(gate);
  state->pending = 0u;
  state->t_change = 0u;
  state->angle = GATE_CLOSED_DEG;
  button_init(gate->btn_port, gate->btn_pin, &state->btn);
}

uint32_t gate_sense(const gate_cfg_t *gate, gate_state_t *state) {
  uint32_t raw = object_detected(gate);

  if(raw == state->detected) {
    state->pending = 0u;
  } else if(!state->pending) {
    state->pending = 1u;
    state->t_change = timer_now();
  } else if(timer_elapsed_ms(state->t_change) >= FC_STABLE_MS) {
    state->detected = raw;
    state->pending = 0u;
  }

  return state->detected;
}

void gate_open(const gate_cfg_t *gate, gate_state_t *state) {
  state->angle = GATE_OPEN_DEG;
  servo_set_angle(gate, state->angle);
  led_set(gate, 1u);
}

void gate_close(const gate_cfg_t *gate, gate_state_t *state) {
  state->angle = GATE_CLOSED_DEG;
  servo_set_angle(gate, state->angle);
  led_set(gate, 0u);
}