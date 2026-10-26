#include "oled.h"
#include "i2c.h"

#define OLED_ADDR 0x3Cu
#define HIGH_OLED 64u
#define OLED_PAGES (HIGH_OLED / 8u)

#define CH_GRADE '\x7F'

typedef struct {
  char c;
  uint8_t col[5];
} glyph_t;

static const glyph_t FONT[] = {
  { ' ',      {0x00,0x00,0x00,0x00,0x00} },
  { '.',      {0x00,0x60,0x60,0x00,0x00} },
  { CH_GRADE, {0x00,0x06,0x09,0x09,0x06} },
  { '0', {0x3E,0x51,0x49,0x45,0x3E} },
  { '1', {0x00,0x42,0x7F,0x40,0x00} },
  { '2', {0x42,0x61,0x51,0x49,0x46} },
  { '3', {0x21,0x41,0x45,0x4B,0x31} },
  { '4', {0x18,0x14,0x12,0x7F,0x10} },
  { '5', {0x27,0x45,0x45,0x45,0x39} },
  { '6', {0x3C,0x4A,0x49,0x49,0x30} },
  { '7', {0x01,0x71,0x09,0x05,0x03} },
  { '8', {0x36,0x49,0x49,0x49,0x36} },
  { '9', {0x06,0x49,0x49,0x29,0x1E} },
  { 'A', {0x7E,0x11,0x11,0x11,0x7E} },
  { 'E', {0x7F,0x49,0x49,0x49,0x41} },
  { 'a', {0x20,0x54,0x54,0x54,0x78} },
  { 'd', {0x38,0x44,0x44,0x48,0x7F} },
  { 'e', {0x38,0x54,0x54,0x54,0x18} },
  { 'g', {0x0C,0x52,0x52,0x52,0x3E} },
  { 'l', {0x00,0x41,0x7F,0x40,0x00} },
  { 'n', {0x7C,0x08,0x04,0x04,0x78} },
  { 'o', {0x38,0x44,0x44,0x44,0x38} },
  { 'p', {0x7C,0x14,0x14,0x14,0x08} },
  { 'r', {0x7C,0x08,0x04,0x04,0x08} },
  { 's', {0x48,0x54,0x54,0x54,0x24} },
  { 'u', {0x3C,0x40,0x40,0x20,0x7C} },
};

static const uint8_t OLED_INIT[] = {
  0xAE,                              /* pantalla apagada */
  0xD5, 0x80,                        /* reloj interno */
  0xA8, HIGH_OLED - 1u,              /* número de filas */
  0xD3, 0x00,                        /* sin desplazamiento vertical */
  0x40,                              /* la memoria empieza en la fila 0 */
  0x8D, 0x14,                        /* bomba de carga interna (genera el voltaje del OLED) */
  0x20, 0x00,                        /* direccionamiento horizontal */
  0xA1,                              /* espejo horizontal (orientación normal del módulo) */
  0xC8,                              /* espejo vertical */
  0xDA, (HIGH_OLED == 64u) ? 0x12 : 0x02,   /* configuración de pines COM */
  0x81, 0xCF,                        /* contraste */
  0xD9, 0xF1,                        /* pre-carga */
  0xDB, 0x40,                        /* nivel VCOMH */
  0xA4,                              /* mostrar el contenido de la memoria */
  0xA6,                              /* sin invertir colores */
  0x2E,                              /* sin scroll */
  0xAF                               /* pantalla encendida */
};

static uint32_t oled_begin(uint8_t control) {
  i2c_start();
  uint32_t ack = i2c_write_byte((uint8_t)(OLED_ADDR << 1));
  i2c_write_byte(control);
  return ack;
}

static void oled_goto(uint32_t col, uint32_t page) {
  oled_begin(0x00);
  i2c_write_byte(0x21);
  i2c_write_byte((uint8_t)col);
  i2c_write_byte(127);
  i2c_write_byte(0x22);
  i2c_write_byte((uint8_t)page);
  i2c_write_byte((uint8_t)(OLED_PAGES - 1u));
  i2c_stop();
}

void oled_clear_page(uint32_t page) {
  oled_goto(0, page);
  oled_begin(0x40);
  for(uint32_t i = 0; i < 128u; ++i) i2c_write_byte(0x00);
  i2c_stop();
}

uint32_t oled_init(void) {
  uint32_t ack = oled_begin(0x00);
  for(uint32_t i = 0; i < sizeof(OLED_INIT); ++i) i2c_write_byte(OLED_INIT[i]);
  i2c_stop();
  for(uint32_t p = 0; p < OLED_PAGES; ++p) oled_clear_page(p);
  return ack;
}

static const uint8_t *glyph(char c) {
  static const uint8_t unknown[5] = { 0X7F, 0x41, 0x41, 0x41, 0x7F };
  for(uint32_t i = 0; i < sizeof(FONT) / sizeof(FONT[0]); ++i)
    if(FONT[i].c == c) return FONT[i].col;

  return unknown;
}

void oled_text_begin(uint32_t col, uint32_t page) {
  oled_goto(col, page);
  oled_begin(0x40);
}

static void oled_text_char(char c) {
  const uint8_t *g = glyph(c);
  for(uint32_t i = 0; i < 5u; ++i) i2c_write_byte(g[i]);
  i2c_write_byte(0x00);
}

void oled_text_str(const char *s) {
  while(*s) oled_text_char(*s++);
}

void oled_text_num(uint32_t n) {
  if(n >= 100u) oled_text_char((char)('0' + n / 100u));
  if(n >= 10u) oled_text_char((char)('0' + (n / 10u) % 10u));
  oled_text_char((char)('0' + n % 10u));
}

void oled_text_end(void) {
  i2c_stop();
}

void oled_bar(uint32_t page, uint32_t angle) {
  uint32_t w = (angle * 126u) / 180u;
  oled_goto(0, page);
  oled_begin(0x40);
  for(uint32_t i = 0; i < 128u; ++i) {
    uint8_t b = (i == 0u || i == 127u || i <= w) ? 0x7E : 0x42;
    i2c_write_byte(b);
  }

  i2c_stop();
}