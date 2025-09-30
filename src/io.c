#include "../includes/io.h"
#include "../includes/timer.h"

#include <stdio.h>

static char dbg;
static uint8_t is_printable;
static uint8_t iflag;

void io_write(uint16_t port, uint8_t value)
{
    switch (port)
    {
        case 0xFF01:        // DBG
            dbg = value;
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
        case 0xFF44:
            return 0x90;
        case 0xFF0F:
            return iflag;
        default:
            break;
    }
    return 0;
}