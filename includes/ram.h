#ifndef RAM__H
#define RAM__H

#include <stdint.h>
#include <stdbool.h>

uint8_t wram_read(uint16_t addr);
uint8_t hram_read(uint16_t addr);
void hram_write(uint16_t addr, uint8_t value);
void wram_write(uint16_t addr, uint8_t value);

#endif