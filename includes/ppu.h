#ifndef PPU__H
#define PPU__H

#include <stdint.h>
#include <stdbool.h>

typedef enum STAT_mode
{
    NONE,
    HBLANK,
    VBLANK,
    OAM_SCAN,
    DRAWING,
}STAT_mode;

void vram_write(uint16_t addr, uint8_t value);
uint8_t vram_read(uint16_t addr);
void ppu_tick();

#endif