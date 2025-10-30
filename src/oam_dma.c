#include "../includes/oam_dma.h"
#include "../includes/timer.h"
#include "../includes/cart.h"
#include "../includes/ram.h"
#include "../includes/io.h"
#include "../includes/cpu.h"
#include "../includes/timer.h"
#include "../includes/ppu.h"

#include <stdio.h>
#include <stdlib.h>

static bool dma_active;
static uint16_t dma;
static uint16_t addr_oam;
static uint16_t addr_dma;
static int bytes_counter;

bool dma_is_active()
{
    return dma_active;
}

uint8_t oam_dma_read()
{
    return dma >> 8;
}
void oam_dma_write(uint8_t value)
{
    dma = value << 8;
    addr_oam = 0xFE00;
    dma_active = true;
    bytes_counter = 0;
    addr_dma = dma;
}

uint8_t dma_bus_read(uint16_t addr, bool update_timer)
{
    if (update_timer)
    {
        // printf("update_timer = true");
        step_timer(1);
    }

    if (addr < 0x8000)          // ROM 32Kib
        return rom_read(addr);
    else if (addr < 0xA000)     // VRAM 8KiB
        return vram_read(addr);
    else if (addr < 0xC000)     // ROM EXT (ROM) 32Kib RAM 4KiB
        return external_ram_read(addr);
    else if (addr < 0xE000)     // WRAM 8Kib
        return wram_read(addr);
    else if (addr < 0xFE00)     // Echo RAM
    {
        // printf("ECHO RAM: Read is prohibited\n");
        // exit(0);
    }
    else if (addr < 0xFEA0)     // OAM
    {
        oam_read(addr);
    }
    else if (addr < 0xFF00)     // Not usable
    {
        // printf("Not usable\n");
        // exit(0);
    }
    else if (addr < 0xFF80)     // IO
        return io_read(addr);
    else if (addr < 0xFFFF)     // HRAM
        return hram_read(addr);
    else if (addr == 0xFFFF)     // IE
        return get_ie();
    else
    {
        printf("READ: Out of range\n");
        exit(0);
    }
    return 0;
}

void dma_bus_write(uint16_t addr, uint8_t value, bool update_timer)
{
    if (update_timer)
        step_timer(1);
    if (addr < 0x8000)          // ROM 32Kib
        rom_write(addr, value);
    else if (addr < 0xA000)     // VRAM 8KiB
        vram_write(addr, value);
    else if (addr < 0xC000)     // RAM 4KiB
        external_ram_write(addr, value);
    else if (addr < 0xE000)     // WRAM 8Kib
        wram_write(addr, value);
    else if (addr < 0xFE00)     // Echo RAM
    {
        // printf("ECHO RAM: Read is prohibited\n");
        // exit(0);
    }
    else if (addr < 0xFEA0)     // OAM
    {
        oam_write(addr, value);
    }
    else if (addr < 0xFF00)     // Not usable
    {
        // printf("Not usable\n");
        // exit(0);
    }
    else if (addr < 0xFF80)     // IO
        io_write(addr, value);
    else if (addr < 0xFFFF)     // HRAM
        hram_write(addr, value);
    else if (addr == 0xFFFF)     // IE
        set_ie(value);
    else
    {
        printf("READ: Out of range\n");
        exit(0);
    }
}

void dma_cycles()
{
    if (dma_active)
    {
        uint8_t data = dma_bus_read(addr_dma++, false);
        dma_bus_write(addr_oam++, data, false);
        bytes_counter++;
        step_timer(1);
        
        // printf("DMA_CYCLES: %do (%02X)\n", bytes_counter, data);

        if (bytes_counter == 160)
            dma_active = false;
        
    }
}