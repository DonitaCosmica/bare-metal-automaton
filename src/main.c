#include <stdint.h>

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
#define PFS_BASE 0x40040800u
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

#define LED_PORT 1
#define LED_PIN 4
#define BTN_PORT 1
#define BTN_PIN 5
#define SRV_PORT 1 // P106 = D4, GTIOC0B (GPT0, salida B)
#define SRV_PIN 6

#define PFS_PODR (1u << 0)
#define PFS_PIDR (1u << 1)
#define PFS_PDR  (1u << 2)
#define PFS_PCR  (1u << 4)
#define PFS_PMR  (1u << 16) // 1 = el pin lo controla un periférico, no el GPIO 
#define PFS_PSEL(n) ((uint32_t)(n) << 24) // qué periférico (bits 28:24)

// PSEL del GPT: 2 = grupo GPT0, 3 = grupo GPT1.
#define SRV_PSEL 3u

// Módulo de reloj del GPT 
#define MSTPCRD (*(volatile uint32_t *)0x40047008)
#define MSTPD5_GPT32 (1u << 5) // 0 = módulo activo; tras el reset está en 1 (apagado)

// GPT0 (timer de 32 bits)
#define GPT0_BASE 0x40078000u
#define GPT0_GTCR (*(volatile uint32_t *)(GPT0_BASE + 0x2C)) // control: CST, modo, divisor 
#define GPT0_GTIOR (*(volatile uint32_t *)(GPT0_BASE + 0x34)) // qué hace el pin
#define GPT0_GTCNT (*(volatile uint32_t *)(GPT0_BASE + 0x48)) // contador
#define GPT0_GTCCRB (*(volatile uint32_t *)(GPT0_BASE + 0x50)) // comparador B = duty 
#define GPT0_GTPR (*(volatile uint32_t *)(GPT0_BASE + 0x64)) // periodo (el "ARR")

#define GTCR_CST (1u << 0)
#define GTCR_TPCS(n) ((uint32_t)(n) << 24)

/* GTIOB[4:0] = 0b11001 = 25:
 *   bit 4 = 1 : nivel inicial ALTO
 *   bits 3:2 = 10 : al fin de ciclo -> ALTO
 *   bits 1:0 = 01 : en compare match con GTCCRB -> BAJO
 * Resultado: pulso alto de ancho GTCCRB al inicio de cada periodo.
 * GTIOB va en los bits 20:16 y OBE (habilitar la salida B) es el bit 24. */
#define GTIOR_GTIOB(v) ((uint32_t)(v) << 16)
#define GTIOR_OBE (1u << 24)

// Matemática del timer
#define PCLKD_HZ 48000000u // PCLKD es el reloj que alimenta al GPT.

#define PWM_TPCS 2u // divisor /16
#define PWM_DIV 16u
#define TIMER_HZ (PCLKD_HZ / PWM_DIV) // fTIM = PCLKD / PSC  -> 3 MHz
#define SERVO_HZ 50u
#define PERIOD_TICKS (TIMER_HZ / SERVO_HZ) // 20 ms -> 60000 cuentas

#define SERVO_MIN_US 544u // pulso para 0 grados (valor por defecto de la librería Servo)
#define SERVO_MAX_US 2400u // pulso para 180 grados 

static void delay(volatile uint32_t n) { while (n--) __asm__("nop"); }

// Configura GPT0 en modo PWM diente de sierra y lo arranca
static void gpt0_pwm_init(uint32_t tpcs, uint32_t period_ticks, uint32_t duty_ticks) {
  MSTPCRD &= ~MSTPD5_GPT32; // 1) activar el módulo GPT32

  GPT0_GTCR = GTCR_TPCS(tpcs); // 2) contador detenido, MD=000 (diente de sierra), divisor 
  GPT0_GTCNT = 0;
  GPT0_GTPR = period_ticks - 1u; // 3) el contador cuenta 0..GTPR, o sea GTPR+1 cuentas 
  GPT0_GTCCRB = duty_ticks; // 4) cuándo baja la salida 
  GPT0_GTIOR = GTIOR_GTIOB(25) | GTIOR_OBE; // 5) comportamiento de la salida B

  PFS(SRV_PORT, SRV_PIN) = 0; // 6) el pin pasa a periférico (primero PMR=0 y luego PSEL)
  PFS(SRV_PORT, SRV_PIN) = PFS_PSEL(SRV_PSEL) | PFS_PMR;

  GPT0_GTCR  |= GTCR_CST; // 7) arrancar
}

static uint32_t angle_to_ticks(uint32_t angle) {
  uint32_t us = SERVO_MIN_US + ((SERVO_MAX_US - SERVO_MIN_US) * angle) / 180u;
  return (us * (TIMER_HZ / 1000u)) / 1000u; // microsegundos -> cuentas
}

static void servo_set_angle(uint32_t angle) {
  GPT0_GTCCRB = angle_to_ticks(angle);
}

int main(void) {
  SCB_VTOR = 0x00004000;

  PWPR = 0x00;
  PWPR = 0x40;

#if CALIBRATION_MODE
  /* Divisor /1024 (TPCS=5): con 48 MHz, 1 cuenta = 21.33 us y 46875 cuentas = 1 s.
   * Duty 50%. Si PCLKD es 48 MHz, el LED hace 0.5 s encendido y 0.5 s apagado. */
  gpt0_pwm_init(5u, PCLKD_HZ / 1024u, (PCLKD_HZ / 1024u) / 2u);
  while (1);
#else
  PFS(BTN_PORT, BTN_PIN) = PFS_PCR;
  delay(1000);

  uint32_t led_on = 0;
  uint32_t angle  = 0;

  PFS(LED_PORT, LED_PIN) = PFS_PDR | (led_on ? PFS_PODR : 0);
  gpt0_pwm_init(PWM_TPCS, PERIOD_TICKS, angle_to_ticks(angle)); // servo en 0 grados

  uint32_t prev = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;

  while (1) {
    uint32_t now = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;

    if (prev == 1 && now == 0) {
      led_on ^= 1;
      PFS(LED_PORT, LED_PIN) = PFS_PDR | (led_on ? PFS_PODR : 0);

      if (led_on) {
        angle = (angle >= 180u) ? 0u : angle + 45u;
        servo_set_angle(angle);
      }

      delay(50000);
    }
    
    prev = now;
  }

#endif

  return 0;
}

void Default_Handler(void) { while (1); }
void Reset_Handler(void) {
  main();
  while (1);
}