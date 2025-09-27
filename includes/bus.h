#ifndef BUS__H
#define BUS__H

#include <stdint.h>

void bus_write(uint16_t addr, uint8_t value);
uint8_t bus_read(uint16_t addr);

#endif