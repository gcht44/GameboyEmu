#include "../includes/io.h"
#include "../includes/timer.h"
#include "../includes/ppu.h"
#include "../includes/lcd.h"
#include "../includes/joypad.h"
#include "../includes/oam_dma.h"

#include <stdio.h>

static char dbg;
static uint8_t is_printable;
static uint8_t iflag;

void io_write(uint16_t port, uint8_t value)
{
    switch (port)
    {
        case 0xFF00:        // JOYPAD
            joypad_write(value);
            break;
        case 0xFF01:        // DBG
            dbg = value;
            break;
        case 0xFF02:        // DBG
            is_printable = value;
            break;
        case 0xFF04:        // DIV
            div_write(value);
            break;
        case 0xFF05:        // TIMA
            tima_write(value);
            break;
        case 0xFF06:        // TMA
            tma_write(value);
            break;
        case 0xFF07:        // TAC
            tac_write(value);
            break;
        case 0xFF40:        // LCDC
            lcdc_write(value);
            break;
        case 0xFF41:        // STAT
            STAT_write(value);
            break;
        case 0xFF42:        // SCY
            SCY_write(value);
            break;   
        case 0xFF43:        // SCX
            SCX_write(value);
            break; 
        case 0xFF45:        // LYC
            lyc_write(value);
            break;
        case 0xFF46:        // DMA
            oam_dma_write(value);
            break;
        case 0xFF47:        // BGP
            bgp_write(value);
            break;  
        case 0xFF48:        // OBP0
            obp0_write(value);
            break;
        case 0xFF49:        // OBP1
            obp1_write(value);
            break;
        case 0xFF4A:        // WY
            WY_write(value);   
            break;
        case 0xFF4B:        // WX  
            WX_write(value);   
            break;
        case 0xFF0F:        // Interrupt flag
            iflag = value;
            break;
        default:
            break;
    }
}
uint8_t io_read(uint16_t port)
{
    switch (port)
    {
        case 0xFF00:        // JOYPAD
            return joypad_read();
        case 0xFF01:
            return dbg;
        case 0xFF02:
            return is_printable;
        case 0xFF04:        // DIV
            return div_read();
        case 0xFF05:        // TIMA
            return tima_read();
        case 0xFF06:        // TMA
            return tma_read();
        case 0xFF07:        // TAC
            return tac_read(); 
        case 0xFF40:        // LCDC
            return lcdc_read();    
        case 0xFF41:        // STAT
            return STAT_read();
        case 0xFF42:        // SCY
            return SCY_read();    
        case 0xFF43:        // SCX
            return SCX_read();        
        case 0xFF44:        // LY
            return read_ly();
        case 0xFF45:        // LYC
            return lyc_read();
        case 0xFF46:        // DMA
            return oam_dma_read();
        case 0xFF47:        // BGP
            return bgp_read();
        case 0xFF48:        // OBP0
            return obp0_read();
        case 0xFF49:        // OBP1
            return obp1_read();
        case 0xFF4A:        // WY
            return WY_read();
        case 0xFF4B:        // WX  
            return WX_read();    
        case 0xFF0F:
            return iflag;
        default:
            break;
    }
    return 0;
}