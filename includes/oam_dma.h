#ifndef OAM_DMA__H
#define OAM_DMA__H

#include <stdint.h>
#include <stdbool.h>

uint8_t oam_dma_read();
void oam_dma_write(uint8_t value);
bool dma_is_active();
void dma_cycles();

#endif