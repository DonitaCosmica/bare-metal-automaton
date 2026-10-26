#include <stdint.h>

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
#define PFS_BASE 0x40040800u
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

#define LED_PORT 1
#define LED_PIN 4
#define FC_51_PORT 1
#define FC_51_PIN 5

#define PFS_PODR (1u << 0)
#define PFS_PIDR (1u << 1)
#define PFS_PDR (1u << 2)
#define PFS_PCR (1u << 4)

#define FC51_LOW_ACTIVE 1

#define MAX_FILTER 20u
#define SHOW_DELAY 500u

static void delay(volatile uint32_t n) {
  while(n--) __asm__ ("nop");
}

static uint32_t object_detected(void) {
  uint32_t level = (PFS(FC_51_PORT, FC_51_PIN) & PFS_PIDR) ? 1u : 0u;
  return FC51_LOW_ACTIVE ? (level ^ 1u) : level;
}

static void led_set(uint32_t on) {
  PFS(LED_PORT, LED_PIN) = PFS_PDR | (on ? PFS_PODR : 0u);
}

int main(void) {
  SCB_VTOR = 0x00004000;

  PWPR = 0x00;
  PWPR = 0x40;

  PFS(FC_51_PORT, FC_51_PIN) = PFS_PCR;
  delay(1000);

  uint32_t detected = object_detected();
  uint32_t count = detected ? MAX_FILTER : 0u;
  led_set(detected);

  while(1) {
    if(object_detected() && count < MAX_FILTER)
      count++;
    else if(count > 0u) {
      count--;
    }

    if(count == MAX_FILTER && !detected) {
      detected = 1;
      led_set(1);
    } else if(count == 0u && detected) {
      detected = 0;
      led_set(0);
    }

    delay(SHOW_DELAY);
  }
}

void Default_Handler(void) { while(1); }
void Reset_Handler(void) {
  main();
  while(1);
}