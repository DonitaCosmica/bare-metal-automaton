#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#define GRADE "\x7F"

uint32_t oled_init(void);
void oled_clear_page(uint32_t);
void oled_text_begin(uint32_t, uint32_t);
void oled_text_str(const char*);
void oled_text_num(uint32_t);
void oled_text_end(void);
void oled_bar(uint32_t, uint32_t);

#endif