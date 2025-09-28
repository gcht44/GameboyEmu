#include "../includes/bus.h"
#include "../includes/cart.h"
#include "../includes/ram.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t bus_read(uint16_t addr)
{
    if (addr < 0x8000)          // ROM 32Kib
        return rom_read(addr);
    else if (addr < 0xA000)     // VRAM 8KiB
    {
        printf("VRAM: Read not implemented\n");
    }
    else if (addr < 0xC000)     // ROM EXT (ROM) 32Kib RAM 4KiB
        return rom_read(addr);
    else if (addr < 0xE000)     // WRAM 8Kib
        return wram_read(addr);
    else if (addr < 0xFE00)     // Echo RAM
    {
        printf("ECHO RAM: Read is prohibited\n");
        exit(0);
    }
    else if (addr < 0xFEA0)     // OAM
    {
        printf("OAM: Read not implemented\n");
        exit(0);
    }
    else if (addr < 0xFF00)     // Not usable
    {
        printf("Not usable\n");
        exit(0);
    }
    else if (addr < 0xFF80)     // IO
    {
        printf("IO: Read not implemented\n");
        exit(0);
    }
    else if (addr < 0xFFFF)     // HRAM
        return hram_read(addr);
    else if (addr == 0xFFFF)     // IE
    {
        printf("IE: Read not implemented\n");
        exit(0);
    }
    else
    {
        printf("READ: Out of range\n");
        exit(0);
    }
    return 0;
}

void bus_write(uint16_t addr, uint8_t value)
{
    if (addr < 0x8000)          // ROM 32Kib
        rom_write(addr, value);
    else if (addr < 0xA000)     // VRAM 8KiB
    {
        printf("VRAM: Read not implemented\n");
        exit(0);
    }
    else if (addr < 0xC000)     // RAM 4KiB
        rom_write(addr, value);
    else if (addr < 0xE000)     // WRAM 8Kib
        wram_write(addr, value);
    else if (addr < 0xFE00)     // Echo RAM
    {
        printf("ECHO RAM: Read is prohibited\n");
        exit(0);
    }
    else if (addr < 0xFEA0)     // OAM
    {
        printf("OAM: Read not implemented\n");
        exit(0);
    }
    else if (addr < 0xFF00)     // Not usable
    {
        printf("Not usable\n");
        exit(0);
    }
    else if (addr < 0xFF80)     // IO
    {
        printf("IO: Read not implemented\n");
        exit(0);
    }
    else if (addr < 0xFFFF)     // HRAM
        return hram_write(addr, value);
    else if (addr == 0xFFFF)     // IE
    {
        printf("IE: Read not implemented\n");
        exit(0);
    }
    else
    {
        printf("READ: Out of range\n");
        exit(0);
    }
}