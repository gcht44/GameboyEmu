#include "../includes/timer.h"
#include "../includes/cpu.h"
#include "../includes/ppu.h"

#include <stdio.h>

static uint16_t div = 0x00CA;
static uint8_t tima;
static uint8_t tma;
static uint8_t tac;
// static bool pending_overflow;
// static uint8_t pending_cycles = -1;

uint8_t div_read()
{
    return div >> 8;
}

void div_write()
{
    div = 0;
}

void div_inc()
{
    div = div + (1 * 4);
}

void tima_write(uint8_t v)
{
    tima = v;
}

uint8_t tima_read()
{
    return tima;
}

void tma_write(uint8_t v)
{
    tma = v;
}

uint8_t tma_read()
{
    return tma;
}

void tac_write(uint8_t v)
{
    tac = v;
}

uint8_t tac_read()
{
    return tac;
}

uint8_t get_bit_watch()
{
    uint8_t clock_select = tac & 0x03;

    if (clock_select == 0) // Increment TIMA every 256 m_cycles
        return 9;
    else if (clock_select == 0b01) // Increment TIMA every 4 m_cycles
        return 3;
    else if (clock_select == 0b10) // Increment TIMA every 16 m_cycles
        return 5;
    else if (clock_select == 0b11) // Increment TIMA every 64 m_cycles
        return 7;
    return 0;
}

void step_timer(uint8_t m_cycles)
{
    uint8_t new_div_bit;
    uint8_t idx = get_bit_watch();
    uint8_t tima_enable = (tac & 0x04) > 0;
    uint8_t last_div_bit = (div>>idx) & 1;

    for (int i=0 ; i < m_cycles ; i++)
    {
        ppu_tick();
        div += 4;
        new_div_bit = (div>>idx) & 1;
        if (tima_enable && (new_div_bit == 0) && (last_div_bit == 1))
        {
            // printf("TIMA ++\n");
            tima++;
            if (tima == 0xFF)
            {
                tima = tma;

                cpu_request_interrupt(IT_TIMER);
            }
        }
        last_div_bit = new_div_bit;
    }
}