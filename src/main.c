#include <stdint.h>

extern uint32_t _estack;
void Reset_Handler(void);
void Default_Handler(void);

/* Tabla de vectores mínima (16 entradas del núcleo) */
__attribute__((section(".isr_vector"), used))
void (*const vectors[16])(void) = {
  (void (*)(void))&_estack, // SP inicial 
  Reset_Handler, // Reset
  Default_Handler, Default_Handler, Default_Handler, Default_Handler,
  Default_Handler, 0, 0, 0, 0,
  Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler
};

#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08)
#define PWPR (*(volatile uint8_t  *)0x40040D03)
#define PFS_BASE 0x40040800u
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

#define LED_PORT 1 // P102 = D13 (LED integrado)
#define LED_PIN  4

#define PFS_PODR (1u << 0) // nivel de salida
#define PFS_PDR  (1u << 2) // 1 = salida

static void delay(volatile uint32_t n) { while (n--) __asm__("nop"); }

int main(void) {
  SCB_VTOR = 0x00004000; // la tabla de vectores está en 0x4000

  PWPR = 0x00; // quitar B0WI 
  PWPR = 0x40; // habilitar PFSWE: permite escribir PFS 

  while (1) {
    PFS(LED_PORT, LED_PIN) = PFS_PDR | PFS_PODR; // ON
    delay(3000000);
    PFS(LED_PORT, LED_PIN) = PFS_PDR; // OFF
    delay(3000000);
  }
}

void Default_Handler(void) { while(1); }
void Reset_Handler(void) { 
  main();
  while (1);
}