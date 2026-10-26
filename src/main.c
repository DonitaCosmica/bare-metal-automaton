#include <stdint.h>

#include "util.h"
#include "led.h"
#include "i2c.h"
#include "oled.h"
#include "servo.h"
#include "ui.h"

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

#define BTN_PORT 1
#define BTN_PIN 5

int main(void) {
  SCB_VTOR = 0x00004000;

  PWPR = 0x00;
  PWPR = 0x40;

#if CALIBRATION_MODE
  gpt0_pwm_init(5u, PCLKD_HZ / 1024u, (PCLKD_HZ / 1024u) / 2u);
  while (1);
#else
  PFS(BTN_PORT, BTN_PIN) = PFS_PCR;
  delay(1000);

  uint32_t led_on = 0;
  uint32_t angle  = 0;

  led_set(led_on);
  gpt0_pwm_init(PWM_TPCS, PERIOD_TICKS, angle_to_ticks(angle));

  i2c_init();
  delay(100000);

  if(!oled_init()) led_blink(5);

  ui_show_wait();
  oled_bar(UI_PAGE_BAR, angle);

  uint32_t prev = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;

  while (1) {
    uint32_t now = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;

    if (prev == 1 && now == 0) {
      led_on ^= 1;
      led_set(led_on);

      if (led_on) {
        angle = (angle >= 180u) ? 0u : angle + 45u;
        servo_set_angle(angle);
        ui_show_angle(angle);
      } else 
        ui_show_wait();

      delay(50000);
    }
    
    prev = now;
  }

#endif

  return 0;
}

void Default_Handler(void) { while(1); }
void Reset_Handler(void) {
  main();
  while (1);
}