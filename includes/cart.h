#ifndef CART__H
#define CART__H

#include <stdint.h>
#include <stdbool.h>

extern int mbc_index;
extern int ext_ram_index;
extern bool ext_ram_active;

int load_rom(const char *path);
uint8_t rom_read(uint16_t addr);
void rom_write(uint16_t addr, uint8_t value);
uint8_t external_ram_read(uint16_t addr);
void external_ram_write(uint16_t addr, uint8_t value);


// MBC
void mbc_1(uint16_t addr, uint8_t value, uint32_t ext_ram_size);
void mbc_5(uint16_t addr, uint8_t value, uint32_t ext_ram_size);

#endif