#include "../includes/cart.h"

int mbc_index = 0x4000;
int ext_ram_index = 0;
bool ext_ram_active = false;

void mbc_5(uint16_t addr, uint8_t value, uint32_t ext_ram_size)
{
    static uint8_t lo = 1;
    static uint8_t ho = 0;
    
    if ((addr <= 0x1FFF)) 
    {
        // EXT RAM ENABLE
        ext_ram_active = ((value & 0x0F) == 0x0A);
    }
    else if ((addr >= 0x2000) && (addr <= 0x2FFF)) 
    {
        // ROM Bank Low 8 bits
        lo = value;
        mbc_index = 0x4000 * ((ho << 8) | lo);
    }
    else if ((addr >= 0x3000) && (addr <= 0x3FFF)) 
    {
        // ROM Bank High bit
        ho = value & 0x01;
        mbc_index = 0x4000 * ((ho << 8) | lo);
    }
    else if ((addr >= 0x4000) && (addr <= 0x5FFF)) 
    {
        // RAM Bank Select
        if (value <= 0x0F) 
        {
            uint32_t new_index = value * 0x2000;
            if (new_index < ext_ram_size) 
            {
                ext_ram_index = new_index;
            }
        }
    }
}

void mbc_1(uint16_t addr, uint8_t value, uint32_t ext_ram_size)
{
    static uint8_t bankselect = 1;
    static int mode = 0; // 0 = default (lock ext ram bank 0) | 1 = advanced

    if ((addr <= 0x1FFF)) 
    {
        // EXT RAM ENABLE
        ext_ram_active = ((value & 0x0F) == 0x0A);
    }
    else if ((addr >= 0x2000) && (addr <= 0x3FFF)) 
    {
        // ROM Bank Low 5 bits
        if (value == 0)
            value += 1;
        bankselect = (bankselect & 0x60) | (value & 0x1F);
        mbc_index = 0x4000 * bankselect;
    }
    else if ((addr >= 0x4000) && (addr <= 0x5FFF)) 
    {
        if (mode == 0)
        {
            // ROM Bank High 2 bits
            bankselect = (bankselect & 0x1F) | (value & 0x60);
            mbc_index = 0x4000 * bankselect;
        }
        else
        {
            // RAM Bank Select
            if (value <= 0x0F) 
            {
                uint32_t new_index = value * 0x2000;
                if (new_index < ext_ram_size) 
                {
                    ext_ram_index = new_index;
                }
            }
        }
    }
    else if ((addr >= 0x6000) && (addr <= 0x7FFF)) 
    {
        mode = value & 0x1;

        if (mode == 1)
            bankselect &= 0x1F;
    }
}