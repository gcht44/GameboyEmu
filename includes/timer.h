#ifndef TIMER__H
#define TIMER__H

#include <stdint.h>
#include <stdbool.h>

void div_write();
uint8_t div_read();
void tima_write(uint8_t v);
uint8_t tima_read();
void tma_write(uint8_t v);
uint8_t tma_read();
void tac_write(uint8_t v);
uint8_t tac_read();

void step_timer(uint8_t m_cycles);

#endif