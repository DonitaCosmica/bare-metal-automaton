# Guía: bare-metal en Arduino UNO R4 WiFi (RA4M1) desde Fedora

Programar en C puro, sin Arduino IDE ni librerías, y flashear por **SWD** con `pyocd`.

- **Placa:** Arduino UNO R4 WiFi (microcontrolador Renesas **RA4M1**, ARM Cortex-M4)
- **Herramientas:** `arm-none-eabi-gcc`, `openocd` (solo para comprobar), `pyocd`
- **Método de flasheo:** SWD a través del CMSIS-DAP integrado (`2341:1002`). **No** se usa el modo bootloader ni `dfu-util`.

> Los pines D2 = P104 y D3 = P105 salen de memoria y no están verificados. Confírmalos en el datasheet o pinout oficial de la UNO R4 WiFi antes de depender de ellos.

---

## 1. Ruta limpia: de cero al LED parpadeando

### 1.1 Herramientas del sistema

```bash
sudo dnf install arm-none-eabi-gcc-cs arm-none-eabi-newlib openocd
```

### 1.2 Archivos del proyecto

Carpeta del proyecto con dos archivos: `ra4m1.ld` y `main.c` (ver secciones 3 y 4).

### 1.3 Compilar

```bash
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -O2 -nostdlib -ffreestanding \
  -T ra4m1.ld main.c -o main.elf
arm-none-eabi-objcopy -O binary main.elf main.bin
arm-none-eabi-size main.elf
```

Revisar el vector de reset (debe empezar con `00 80 00 20`):

```bash
hexdump -C main.bin | head -n 1
```

### 1.4 Comprobar que el SWD ve el chip (una sola vez)

```bash
sudo openocd -f interface/cmsis-dap.cfg \
  -c "transport select swd" -c "adapter speed 1000" \
  -c "swd newdap ra4m1 cpu -enable" \
  -c "dap create ra4m1.dap -chain-position ra4m1.cpu" \
  -c "target create ra4m1.cpu cortex_m -dap ra4m1.dap" \
  -c "init" -c "targets" -c "shutdown"
```

Éxito: `SWD DPIDR 0x5ba02477`, `Cortex-M4 r0p1 processor detected`, `Examination succeed`.

### 1.5 Entorno virtual con pyocd (dentro del proyecto)

```bash
cd ~/Documents/School/Automatas/Project_2
python3 -m venv .venv-pyocd
source .venv-pyocd/bin/activate
pip install pyocd
```

- Agregar `.venv-pyocd/` al `.gitignore` si se usa git.
- En cada terminal nueva hay que repetir `source .venv-pyocd/bin/activate`.

### 1.6 Instalar el pack del chip (una sola vez)

```bash
pyocd pack find r7fa4m1          # debe listar R7FA4M1AB, Installed: False
pyocd pack install r7fa4m1ab
pyocd pack find r7fa4m1          # ahora Installed: True
pyocd list --targets | grep -i r7fa4m1
```

El nombre de target a usar es **`r7fa4m1ab`**. El pack se guarda en la caché del usuario, no dentro de `.venv-pyocd`.

### 1.7 Flashear

Placa conectada normal, **sin doble reset**:

```bash
pyocd flash -t r7fa4m1ab --base-address 0x4000 main.bin
```

Éxito: `Erased 2048 bytes (1 sector), programmed 2048 bytes (1 page)`. Después, un clic en RESET o reconectar el USB.

> `--base-address 0x4000` debe coincidir con el `ORIGIN` del linker script y con el valor de `SCB_VTOR`.

### 1.8 Cada cambio de código

```bash
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -O2 -nostdlib -ffreestanding -T ra4m1.ld main.c -o main.elf
arm-none-eabi-objcopy -O binary main.elf main.bin
pyocd flash -t r7fa4m1ab --base-address 0x4000 main.bin
```

Opcional, guardarlo en un `flash.sh` con `set -e` al inicio.

### 1.9 Limpieza del intento con ESP32

Borrar `esp32s3.ld` y la configuración "ESP32-S3 Bare-Metal" de `.vscode/c_cpp_properties.json`. Usar `/usr/bin/arm-none-eabi-gcc` como `compilerPath` para IntelliSense.

---

## 2. Errores que ocurrieron, causa y solución

| # | Síntoma | Causa | Solución |
|---|---|---|---|
| 1 | `esptool`: `Failed to connect to ESP32-S3: No serial data received` | Toolchain y herramienta del chip equivocado: el UNO R4 usa un RA4M1 (ARM), no un ESP32-S3 | Usar `arm-none-eabi-gcc`, un linker nuevo y los registros del RA4M1 |
| 2 | `dfu-util -l` vacío; `lsusb` solo ve `2341:1002` | La placa estaba en modo normal (CMSIS-DAP) y el bootloader DFU no apareció, aunque el LED "L" latía | Abandonar el DFU y flashear por SWD |
| 3 | `device descriptor read/64, error -71` en `dmesg` | Enumeración USB inestable (cable o puerto marginal) | Cambiar de cable; con el nuevo enumeró limpio |
| 4 | OpenOCD: `gdb services need one or more targets defined` | Primera prueba sin target definido; no era un fallo del chip | Usar el comando de la sección 1.4 |
| 5 | `pyocd pack find ra4m1`: `No matching devices` | `pack find` busca por nombre de parte (R7FA4M1AB) | `pyocd pack find r7fa4m1` |
| 6 | `pyocd pack find renesas`: `No matching devices` | Se busca por parte, no por fabricante | Buscar por nombre de parte |
| 7 | `xxd: command not found` | No viene instalado | `hexdump -C main.bin \| head -n 1` o `sudo dnf install xxd` |
| 8 | El LED arrancaba encendido con `led_on = 0` | La primera lectura del botón daba 0 (el pull-up aún no subía) y generaba un flanco falso | `delay(1000)` tras activar el pull-up y calcular `prev` con una lectura real |
| 9 | `PWPR` definido como `uint32_t` | Es un registro de 8 bits en una dirección no alineada; el acceso de 32 bits toca bytes vecinos | Declararlo como `volatile uint8_t *` |

### Cosas que se hicieron y no hacen falta

- `sudo usermod -a -G dialout $USER` y `esptool`: eran del camino ESP32.
- `dfu-util` y el doble reset: no funcionaron en esta placa. El ID del bootloader de la R4 WiFi nunca se verificó (se mencionaron `2341:0369` y `2341:0069` sin confirmarlos).
- Modo bootloader al flashear: no se necesita con pyocd.

### Si algo falla en el futuro

- **pyocd no conecta:** `pyocd flash -t r7fa4m1ab -O connect_mode=under-reset --base-address 0x4000 main.bin`
- **Falta de permisos USB sin `sudo`:** hace falta una regla udev (en este caso no fue necesaria).
- **No usar** `pyocd erase --chip`: borraría el bootloader de Arduino.
- **El LED no hace lo esperado:** inspeccionar el registro con `pyocd commander -t r7fa4m1ab` y `read32 0x40040854` (botón en P105).

---

## 3. Linker script: `ra4m1.ld`

```ld
ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00004000, LENGTH = 240K   /* 256K - 16K del bootloader */
  RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .text : {
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
  } > FLASH
}
```

---

## 4. Programa: LED que sigue un botón con toggle (`main.c`)

LED externo en D2 (P104) y botón a GND en D3 (P105) con pull-up interno.

```c
#include <stdint.h>

extern uint32_t _estack;
void Reset_Handler(void);
void Default_Handler(void) { while (1); }

/* Tabla de vectores mínima (16 excepciones del núcleo) */
__attribute__((section(".isr_vector"), used))
void (*const vectors[16])(void) = {
  (void (*)(void))&_estack,   /* SP inicial */
  Reset_Handler,              /* Reset */
  Default_Handler, Default_Handler, Default_Handler, Default_Handler,
  Default_Handler, 0, 0, 0, 0,
  Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler
};

#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08)   /* dónde está la tabla de vectores */
#define PWPR     (*(volatile uint8_t  *)0x40040D03)   /* protección de escritura de los PFS (8 bits) */

#define PFS_BASE 0x40040800u
/* Macro: designa el registro de 32 bits del pin. A la derecha de '=' lee; a la izquierda escribe. */
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

#define LED_PORT 1   /* P104 = D2 */
#define LED_PIN  4
#define BTN_PORT 1   /* P105 = D3 */
#define BTN_PIN  5

#define PFS_PODR (1u << 0)   /* nivel de salida (1 = alto) */
#define PFS_PIDR (1u << 1)   /* nivel leído del pin (1 = alto, 0 = bajo); lo actualiza el hardware */
#define PFS_PDR  (1u << 2)   /* dirección (0 = entrada, 1 = salida) */
#define PFS_PCR  (1u << 4)   /* 1 = pull-up interno */

static void delay(volatile uint32_t n) { while (n--) __asm__("nop"); }

int main(void) {
  SCB_VTOR = 0x00004000;   /* la tabla está en 0x4000, no en 0 (el bootloader ocupa 0x0-0x3FFF) */

  PWPR = 0x00;             /* bit 7 (B0WI) a 0: permite modificar PFSWE */
  PWPR = 0x40;             /* bit 6 (PFSWE) a 1: permite escribir los PFS */

  PFS(BTN_PORT, BTN_PIN) = PFS_PCR;   /* entrada con pull-up; los demás campos en 0 (GPIO) */
  delay(1000);                        /* deja subir el pull-up antes de leer */

  uint32_t led_on = 0;
  PFS(LED_PORT, LED_PIN) = PFS_PDR | (led_on ? PFS_PODR : 0);
  uint32_t prev = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;   /* 1 = suelto, 0 = presionado */

  while (1) {
    uint32_t now = (PFS(BTN_PORT, BTN_PIN) & PFS_PIDR) ? 1 : 0;

    if (prev == 1 && now == 0) {      /* flanco de bajada: suelto -> presionado */
      led_on ^= 1;                    /* alterna: 0 -> 1, 1 -> 0 */
      PFS(LED_PORT, LED_PIN) = PFS_PDR | (led_on ? PFS_PODR : 0);
      delay(50000);                   /* antirrebote tosco; depende del reloj heredado */
    }
    prev = now;
  }
}

void Reset_Handler(void) { main(); while (1); }
```

Para un simple parpadeo, reemplaza el `while (1)` por:

```c
while (1) {
  PFS(LED_PORT, LED_PIN) = PFS_PDR | PFS_PODR;  /* ON  */
  delay(1000000);
  PFS(LED_PORT, LED_PIN) = PFS_PDR;             /* OFF */
  delay(1000000);
}
```

Si agregas variables globales, tendrás que copiar `.data` y poner `.bss` en cero en `Reset_Handler`; este programa no las usa.

---

## 5. Referencia rápida de la arquitectura

### 5.1 AVR (R3) vs Cortex-M (R4)

| | ATmega328P (R3) | RA4M1 (R4) |
|---|---|---|
| Núcleo | AVR de 8 bits | ARM Cortex-M4 de 32 bits |
| Registros de I/O | Pocos, de 8 bits (`DDRB`, `PORTB`, `PINB`) | Registros de 32 bits mapeados en memoria |
| Pines | 8 pines por puerto, un registro por puerto | Hasta 16 pines por puerto, un registro **por pin** |
| Dirección del pin | `DDRB`: 1 = salida | Bit `PDR` del `PFS` del pin |
| Arranque | `jmp` en 0x0000 | Tabla de direcciones (la primera es el stack) |
| Reloj | 16 MHz fijo | Configurable (aquí se hereda el del bootloader) |

### 5.2 Mapa de memoria

| Región | Dirección | Contenido |
|---|---|---|
| Flash | `0x00000000` | Bootloader (16 KB) y tu programa desde `0x4000` |
| SRAM | `0x20000000` | 32 KB (final en `0x20008000`, valor inicial del stack) |
| Periféricos | `0x40000000` | GPIO, timers, UART... |
| Núcleo ARM | `0xE0000000` | SysTick, NVIC, SCB (incluye `VTOR` en `0xE000ED08`) |

### 5.3 Nombre de un pin y dirección de su registro

`P` + puerto + pin de dos dígitos: **P104** = puerto 1, pin 04.

```
dirección = 0x40040800 + 0x40*puerto + 4*pin
P104 -> 0x40040800 + 0x40 + 0x10 = 0x40040850
P105 -> 0x40040854
```

### 5.3.1 Bits del registro PFS usados

| Bit | Nombre | Significado | Quién lo cambia |
|---|---|---|---|
| 0 | `PODR` | Nivel de salida | Tú |
| 1 | `PIDR` | Nivel leído del pin | El hardware |
| 2 | `PDR` | 0 = entrada, 1 = salida | Tú |
| 4 | `PCR` | Pull-up interno | Tú |

Valores de uso común:

- Salida alta: `PDR | PODR = 0b101 = 0x5`
- Salida baja: `PDR = 0b100 = 0x4`
- Entrada con pull-up: `PCR = 0x10`. El registro leído vale `0x12` (suelto, pin en alto) o `0x10` (presionado, pin en bajo).

### 5.4 Cómo se lee el botón (con binarios)

```
Botón suelto (registro 0x12):
registro:        ...0001 0010
PFS_PIDR (0x2):  ...0000 0010
AND (&)          ...0000 0010  = 2  (distinto de 0 -> pin en ALTO -> suelto)

Botón presionado (registro 0x10):
registro:        ...0001 0000
PFS_PIDR (0x2):  ...0000 0010
AND (&)          ...0000 0000  = 0  (pin en BAJO -> presionado)
```

`PFS_PIDR` es solo una máscara constante (`0x2`) que aísla el bit 1. La lectura la hace el acceso al registro; `volatile` obliga al compilador a releerlo en cada vuelta del bucle.

### 5.5 Cómo se prueba un pin sin recompilar

```bash
pyocd commander -t r7fa4m1ab
```

Dentro (ejemplo con P104, D2):

```
halt
write8 0x40040D03 0x00
write8 0x40040D03 0x40
write32 0x40040850 0x5     # LED en D2 ON
write32 0x40040850 0x4     # LED en D2 OFF
go
```

### 5.6 Cuándo usar ensamblador

Casi siempre en una línea (*inline asm*), cuando C no puede expresar la instrucción:

- `cpsid i` / `cpsie i`: deshabilitar y habilitar interrupciones
- `wfi` / `wfe`: dormir el núcleo
- `dsb` / `isb`: barreras de memoria (por ejemplo, tras escribir `VTOR`)
- `mrs` / `msr`: registros especiales del núcleo (`PRIMASK`, `MSP`, `PSP`, `CONTROL`)
- Cambio de contexto de un RTOS (`PendSV`)
- Tiempos medidos en ciclos exactos

Para ver lo que genera el compilador: `arm-none-eabi-objdump -d main.elf`

### 5.7 Pines que conviene no tocar

- Pines de depuración SWD (según el datasheet del chip, `P108` y `P300`): si los cambias a GPIO, pierdes la conexión de pyocd.
- El LED integrado (`P102`), el USB y los pines que comunican con el ESP32-S3.
- Usa los de las cabeceras D2–D12 y A0–A5, y confirma en el datasheet que no tengan uso reservado.

---

## 6. Dónde buscar cada dato

- **Pinout y correspondencia D/A ↔ Pxxx:** datasheet y pinout de la UNO R4 WiFi en docs.arduino.cc
- **Correspondencia en código:** repositorio `ArduinoCore-renesas`, carpeta `variants/UNOWIFIR4/` (verificar el nombre exacto del archivo)
- **Registros y funciones de cada pin:** *RA4M1 Group User's Manual: Hardware* de Renesas (capítulo I/O Ports)

## 7. Siguientes pasos de aprendizaje

1. Reemplazar los números mágicos por `struct`s de registros.
2. Retardos reales con el timer SysTick (registros desde `0xE000E010`).
3. Configurar el reloj y una interrupción externa por botón.
4. Usar un UART.

---

## Uso en este proyecto

Por medio del archivo Makefile se puede compilar el proyecto de manera mas sencilla

- make: Compila el proyecto general para crear un archivo binario.
- make flash: Se encarga de flashear el codigo en la placa Arduino.
- make clean: Se encarga de limpiar el proyecto. Borrar los archivos .elf .bin para volver a compilar el proyecto.