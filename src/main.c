#include <stdint.h>

extern uint32_t _estack;
void Reset_Handler(void);
void Default_Handler(void) { while(1); };

__attribute__((section(".isr_vector"), used))
void(*const vectors[16])(void) = {
  (void (*)(void))&_estack,
  Reset_Handler,
  Default_Handler, Default_Handler, Default_Handler, Default_Handler,
  Default_Handler, 0, 0, 0, 0,
  Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler
};

// la tabla de vectores está en 0x4000, no en 0 (el bootloader ocupa 0x0-0x3FFF)
#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08)
#define PWPR (*(volatile uint8_t *)0x40040D03)
/* Dirección del registro PFS del P000 (inicio de los registros de configuración de pines) */
#define PFS_BASE 0x40040800u
/* PFS(PORT, PIN) lo que hace es obtener el valor de la direccion de memoria calculada (Una funcion que retorna un numero binario que esta en una direccion de memoria)*/
/* PFS(port, pin): macro que apunta al registro de 32 bits del pin (dirección calculada).
   Al leerla se obtiene su valor; al asignarle algo se escribe en el registro. */
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

// P104 -> Puerto 1, Pin 4 | P012 -> Puerto 0, Pin 12
#define LED_PORT 1
#define LED_PIN 4
#define BTN_PORT 1
#define BTN_PIN 5

#define PFS_PODR (1u << 0) // Salida (0 = OFF, 1 = ON)
#define PFS_PIDR (1u << 1)  // Nivel leído del pin (1 = alto, 0 = bajo). Lo actualiza el hardware
#define PFS_PDR (1u << 2) // Direccion del pin (0 = entrada, 1 Salida)
#define PFS_PCR (1u << 4) // Pull-up interno (1 = pull-up interno activado)

static void delay(volatile uint32_t n) {
  while(n--) __asm__("nop");
}

int main(void) {
  SCB_VTOR = 0X00004000;

  PWPR = 0x00; // bit 7 (B0WI) a 0: permite modificar PFSWE
  PWPR = 0x40; // bit 6 (PFSWE) a 1: permite escribir los PFS

  PFS(BTN_PORT, BTN_PIN) = PFS_PCR;
  delay(1000);

  // uint32_t reg = PFS(BTN_PORT, BTN_PIN); lee los 32 bits: 0x12 o 0x10
  // uint32_t bit = reg & PFS_PIDR; 2) aísla el bit 1: 0x2 o 0x0
  // uint32_t now = bit ? 1 : 0; lo normaliza a 1 o 0

  uint32_t led_on = 0;
  PFS(LED_PORT, LED_PIN) = PFS_PDR | (led_on ? PFS_PODR : 0);
  uint32_t prev = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0; // 1 = boton no presionado , 0 = boton presionado 

  while(1) {
    uint32_t now = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;

    if(prev == 1 && now == 0) {
      led_on ^= 1; // alterna (toggle): 0 -> 1, 1 -> 0
      PFS(LED_PORT, LED_PIN) = PFS_PDR | (led_on ? PFS_PODR : 0);
      delay(50000);
    }

    prev = now;
  }

  return 0;
}

void Reset_Handler(void) {
  main();
  while(1);
}