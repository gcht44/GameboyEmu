#ifndef IO__H
#define IO__H

#include <stdint.h>
#include <stdbool.h>

void io_write(uint16_t port, uint8_t value);
uint8_t io_read(uint16_t port);

#endif