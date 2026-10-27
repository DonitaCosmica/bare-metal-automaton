#include "i2c.h"
#include "util.h"

#define SDA_PORT 1 // P101 - A4
#define SDA_PIN 1
#define SCL_PORT 1 // P100 - A5
#define SCL_PIN 0

#define I2C_DELAY 20u

static void i2c_delay(void) {
  delay(I2C_DELAY);
}

static void scl_low(void) {
  PFS(SCL_PORT, SCL_PIN) = PFS_PDR;
}

static void scl_high(void) {
  PFS(SCL_PORT, SCL_PIN) = 0u;
}

static void sda_low(void) {
  PFS(SDA_PORT, SDA_PIN) = PFS_PDR;
}

static void sda_high(void) {
  PFS(SDA_PORT, SDA_PIN) = 0u;
}

static uint32_t sda_read(void) {
  return (PFS(SDA_PORT, SDA_PIN) & PFS_PIDR) ? 1u : 0u;
}

void i2c_init(void) {
  sda_high();
  scl_high();
  i2c_delay();
}

void i2c_start(void) {
  sda_high(); scl_high(); i2c_delay();
  sda_low(); i2c_delay();
  scl_low(); i2c_delay();
}

void i2c_stop(void) {
  sda_low(); i2c_delay();
  scl_high(); i2c_delay();
  sda_high(); i2c_delay();
}

uint32_t i2c_write_byte(uint8_t data) {
  for(uint32_t mask = 0x80u; mask != 0u; mask >>= 1) {
    if(data & mask) sda_high();
    else sda_low();

    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
  }

  sda_high();
  i2c_delay();
  scl_high();
  i2c_delay();
  uint32_t ack = (sda_read() == 0u);
  scl_low();
  return ack;
}