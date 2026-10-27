#include "fc.h"

uint32_t object_detected(const gate_cfg_t *unit) {
  uint32_t level = (PFS(unit->fc_port, unit->fc_pin) & PFS_PIDR) ? 1u : 0u;
  return FC_LOW_ACTIVE ? (level ^ 1u) : level;
}