#ifndef GATE_H
#define GATE_H

#include <stdint.h>
#include "util.h"

#define GATE_CLOSED_DEG 0u
#define GATE_OPEN_DEG 90u

void gate_init(const gate_cfg_t*, gate_state_t*);
uint32_t gate_sense(const gate_cfg_t*, gate_state_t*);
void gate_open(const gate_cfg_t*, gate_state_t*);
void gate_close(const gate_cfg_t*, gate_state_t*);

#endif