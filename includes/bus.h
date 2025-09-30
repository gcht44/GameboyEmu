#ifndef BUS__H
#define BUS__H

#include <stdint.h>
#include <stdbool.h>

void bus_write(uint16_t addr, uint8_t value, bool update_timer);
uint8_t bus_read(uint16_t addr, bool update_timer);

#endif