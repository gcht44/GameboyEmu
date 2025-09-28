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
} s_CPU;

#endif