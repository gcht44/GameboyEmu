#include <stdio.h>

#include "../includes/cart.h"
#include "../includes/cpu.h"
#include "../includes/bus.h"

int main()
{
    s_CPU cpu;
    bool play_emu = true;

    load_rom("rom/test_roms/cpu_instr/02-interrupts.gb");
    init_cpu(&cpu);    

    FILE *fp = fopen("log/log.txt", "w");

    while(play_emu)
    {
        fprintf(fp, "%04X:  A:%02x F:%02x B:%02x C:%02x D:%02x E:%02x H:%02x L:%02x  SP:%04x PCMEM:%02X,%02X,%02X,%02X IF:%02X IE:%02X IME:%d TIMA:%02X DIV:%04X\n",
        cpu.pc, cpu.a, (((cpu.zf << 7) | (cpu.nf << 6) | (cpu.hf << 5) | (cpu.cy << 4)) & 0xF0), cpu.b, cpu.c, cpu.d, cpu.e, cpu.h, cpu.l,
        cpu.sp, bus_read(cpu.pc, false), bus_read(cpu.pc+1, false), bus_read(cpu.pc+2, false), bus_read(cpu.pc+3, false), bus_read(0xFF0F, false), bus_read(0xFFFF, false), 
        cpu.ime, bus_read(0xFF05, false), bus_read(0xFF04, false));
        step_emu(&cpu);
    }
    fclose(fp);
    return 0;
}