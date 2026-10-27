#ifndef UI_H
#define UI_H

#include <stdint.h>

void ui_init(void);
void ui_show_message(const char*);
void ui_show_parked(uint32_t, uint32_t);
void ui_show_ticket_info(uint32_t, uint32_t, uint32_t);
void ui_show_money_paid(uint32_t, uint32_t);
void ui_show_paid(uint32_t, uint32_t);
void ui_show_refund(const char*, uint32_t); 

#endif