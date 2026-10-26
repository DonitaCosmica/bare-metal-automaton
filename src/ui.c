#include "ui.h"
#include "oled.h"

#define UI_COL_TEXT 10u
#define UI_PAGE_TEXT 2u

void ui_show_angle(uint32_t angle) {
  oled_clear_page(UI_PAGE_TEXT);
  oled_text_begin(UI_COL_TEXT, UI_PAGE_TEXT);
  oled_text_str("Angulo en ");
  oled_text_num(angle);
  oled_text_str(GRADE);
  oled_text_end();
  oled_bar(UI_PAGE_BAR, angle);
}

void ui_show_wait(void) {
  oled_clear_page(UI_PAGE_TEXT);
  oled_text_begin(UI_COL_TEXT, UI_PAGE_TEXT);
  oled_text_str("Esperando...");
  oled_text_end();
}