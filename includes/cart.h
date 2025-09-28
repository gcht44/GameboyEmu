#ifndef CART__H
#define CART__H

#include <stdint.h>

int load_rom(const char *path);
uint8_t rom_read(uint16_t addr);
void rom_write(uint16_t addr, uint8_t value);

#endif