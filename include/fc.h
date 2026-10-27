#ifndef FC_H
#define FC_H

#include <stdint.h>
#include "util.h"

#define FC_ENTRANCE_PORT 3 // P301 - D0
#define FC_ENTRANCE_PIN 1
#define FC_EXIT_PORT 3 // P304 - D8
#define FC_EXIT_PIN 4

#define FC_LOW_ACTIVE 1
#define FC_STABLE_MS 30u
#define SHOW_DELAY 500u

uint32_t object_detected(const gate_cfg_t*);

#endif