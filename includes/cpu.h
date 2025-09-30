#ifndef CPU__H
#define CPU__H

#include <stdint.h>
#include <stdbool.h>

typedef struct CPU
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;

    uint16_t pc;
    uint16_t sp;

    bool zf;
    bool nf;
    bool hf;
    bool cy;
    
    bool ime;
    bool halt;
    bool halt_bug;
    bool ime_pending;
} s_CPU;

typedef enum {
    IT_VBLANK = 1,
    IT_LCD_STAT = 2,
    IT_TIMER = 4,
    IT_SERIAL = 8,
    IT_JOYPAD = 16
} s_Int_Type;

void init_cpu(s_CPU *c);
void step_emu(s_CPU *c);
unsigned long get_cyc();
uint8_t get_ie();
void set_ie(uint8_t v);
void cpu_request_interrupt(s_Int_Type it);

#endif