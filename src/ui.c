#include "ui.h"
#include "i2c.h"
#include "oled.h"
#include "led.h"
#include "util.h"

#define UI_COL_TEXT 2u
#define UI_PAGE_TEXT 2u
#define UI_PAGE_BAR 7u

static void ui_text(uint32_t page, const char *s) {
  oled_text_begin(UI_COL_TEXT, page);
  oled_text_str(s);
  oled_text_end();
}

static void ui_text_num(uint32_t page, const char *label, uint32_t n, const char *suffix) {
  oled_text_begin(UI_COL_TEXT, page);
  oled_text_str(label);
  oled_text_num(n);
  oled_text_str(suffix);
  oled_text_end();
}

void ui_init(void) {
  i2c_init();
  delay(100000);

  if(!oled_init()) led_blink(5);
}

void ui_show_message(const char *message) {
  oled_clear_all();
  oled_text_multiline(UI_COL_TEXT, UI_PAGE_TEXT, message);
}

void ui_show_parked(uint32_t occupied, uint32_t capacity) {
  oled_clear_all();
  oled_text_begin(UI_COL_TEXT, 2u);
  oled_text_str("Q4: Cupo: ");
  oled_text_num(occupied);
  oled_text_str("/");
  oled_text_num(capacity);
  oled_text_end();
  ui_text(5u, "Para pagar:");
  ui_text(6u, "inserte boleto");
}

void ui_show_ticket_info(uint32_t serial, uint32_t hours, uint32_t total) {
  uint32_t tens = total / 10u;
  uint32_t five = total % 10u;

  oled_clear_all();
  ui_text(0u, "Q5: PAGO DE BOLETO");
  ui_text_num(2u, "Serie: ", serial, "");
  ui_text_num(3u, "Tiempo: ", hours, " h");
  ui_text_num(4u, "Precio: $", total, "");

  oled_text_begin(UI_COL_TEXT, 5u);
  oled_text_str("Monedas: ");
  oled_text_num(tens);
  oled_text_str("x$10");
  if(five) oled_text_str(" + $5");
  oled_text_end();

  ui_text(6u, "Inserte monedas");
  oled_bar(UI_PAGE_BAR, 0u, total);
}

void ui_show_money_paid(uint32_t total, uint32_t paid) {
  uint32_t missing = (total > paid) ? (total - paid) : 0u;

  oled_clear_page(6u);
  ui_text_num(6u, "Q6: Falta: $", missing, "");
  oled_bar(UI_PAGE_BAR, paid, total);
}

void ui_show_paid(uint32_t paid, uint32_t change) {
  oled_clear_all();
  ui_text(1u, "Q7: PAGO COMPLETO");
  ui_text_num(3u, "Pagado: $", paid, "");
  ui_text_num(4u, "Cambio: $", change, "");
  ui_text(6u, "Salida autorizada");
  ui_text(7u, "Tiene 15 s");
}

void ui_show_refund(const char *reason, uint32_t refund) {
  oled_clear_all();
  ui_text(2u, reason);
  ui_text(3u, "Boleto devuelto");
  ui_text_num(4u, "Dinero: $", refund, "");
}