#include "../includes/ram.h"
#include "../includes/cpu.h"
#include "../includes/bus.h"

static uint8_t wram[0x2000];
static uint8_t hram[0x80];

uint8_t wram_read(uint16_t addr)
{
    addr -= 0xC000;
    return wram[addr];
}

uint8_t hram_read(uint16_t addr)
{
    addr -= 0xFF80;
    return hram[addr];
}

void hram_write(uint16_t addr, uint8_t value)
{
    addr -= 0xFF80;
    hram[addr] = value;
}

void wram_write(uint16_t addr, uint8_t value)
{
    addr -= 0xC000;
    wram[addr] = value;
}