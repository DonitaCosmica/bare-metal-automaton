#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define PFS_BASE 0x40040800u
#define PFS(port, pin) (*(volatile uint32_t *)(PFS_BASE + 0x40u*(port) + 4u*(pin)))

#define PFS_PODR (1u << 0)
#define PFS_PIDR (1u << 1)
#define PFS_PDR  (1u << 2)
#define PFS_PCR  (1u << 4)
#define PFS_PMR  (1u << 16)
#define PFS_PSEL(n) ((uint32_t)(n) << 24)

void delay(volatile uint32_t);

#endif