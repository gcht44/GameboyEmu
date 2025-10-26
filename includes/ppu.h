#ifndef PPU__H
#define PPU__H

#include <stdint.h>
#include <stdbool.h>

typedef enum STAT_mode
{
    HBLANK,
    VBLANK,
    OAM_SCAN,
    DRAWING,
}STAT_mode;

void vram_write(uint16_t addr, uint8_t value);
uint8_t vram_read(uint16_t addr);
void ppu_tick();
void lyc_write(uint8_t value);
uint8_t lyc_read();
void STAT_write(uint8_t value);
uint8_t STAT_read();
uint8_t read_ly();
void SCX_write(uint8_t value);
uint8_t SCX_read();
void SCY_write(uint8_t value);
uint8_t SCY_read();
void WY_write(uint8_t value);
uint8_t WY_read();
void WX_write(uint8_t value);
uint8_t WX_read();
void oam_write(uint16_t addr, uint8_t v);
uint8_t oam_read(uint16_t addr);


#endif