#ifndef RAM__H
#define RAM__H

#include <stdint.h>
#include <stdbool.h>

typedef struct s_CPU s_CPU;

uint8_t wram_read(uint16_t addr);
uint8_t hram_read(uint16_t addr);
void hram_write(uint16_t addr, uint8_t value);
void wram_write(uint16_t addr, uint8_t value);
void push(s_CPU *c, uint8_t value);
uint8_t pop(s_CPU *c);

#endif