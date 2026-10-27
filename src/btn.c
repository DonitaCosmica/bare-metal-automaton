#include "btn.h"
#include "timebase.h"

uint32_t button_read(uint8_t port, uint8_t pin) {
  return (PFS(port, pin) & PFS_PIDR) ? 1u : 0u;
}

void button_init(uint8_t port, uint8_t pin, btn_state_t *btn) {
  PFS(port, pin) = PFS_PCR;
  delay(1000);

  btn->prev = button_read(port, pin);
  btn->t_last = 0u;
  btn->t_last = 0u;
}

uint32_t button_pressed(uint8_t port, uint8_t pin, btn_state_t *btn) {
  uint32_t now = button_read(port, pin);
  uint32_t pressed = 0u;

  if(btn->prev == 1u && now == 0u) {
    if(!btn->used || timer_elapsed_ms(btn->t_last) >= BTN_DEBOUNCE_MS) {
      pressed = 1u;
      btn->t_last = timer_now();
      btn->used = 1u;
    }
  }

  btn->prev = now;
  return pressed;
}