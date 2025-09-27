#include "../includes/cpu.h"
#include "../includes/bus.h"


void init_cpu(s_CPU *c)
{
    c->a = 0;
    c->b = 0;
    c->c = 0;
    c->d = 0;
    c->e = 0;
    c->h = 0;
    c->l = 0;

    c->pc = 0x100;
    c->sp = 0;
    
    c->zf = 0;
    c->hf = 0;
    c->nf = 0;
    c->cy = 0;
}

void cpu_ld(bool dest_is_mem, uint16_t addr, uint8_t *dst, uint8_t src)
{
    if (dest_is_mem)
        bus_write(addr, src);
    else
        *dst = src;

}

uint8_t cpu_inc(s_CPU *c, uint8_t val)
{
    uint8_t res = val + 1;
    if (res == 0)
        c->zf = 1;
    else
        c->zf = 0;
    if (((val & 0x0F) + 1) > 0x0F)
        c->hf = 1;
    else
        c->hf = 0;
    c->nf = 0;
    return res;
}

void cpu_inc16(uint8_t *hi, uint8_t *lo)
{
    (*lo)++;
    if (*lo == 0x00)
    {
        (*hi)++;
    }
}

uint8_t cpu_dec(s_CPU *c, uint8_t val)
{
    uint8_t res = val - 1;
    if (res == 0)
        c->zf = 1;
    else
        c->zf = 0;
    if ((((int8_t)val & 0x0F) - 1) < 0)
        c->hf = 1;
    else
        c->hf = 0;
    c->nf = 1;
    return res;
}

void cpu_dec16(uint8_t *hi, uint8_t *lo)
{
    (*lo)--;
    if (*lo == 0xFF)
    {
        (*hi)--;
    }
}

void cpu_add16(s_CPU *c, uint8_t *dst_hi, uint8_t *dst_lo, uint8_t src_hi, uint8_t src_lo)
{
    uint16_t dst = (*dst_hi << 8) | *dst_lo;
    uint16_t src = (src_hi << 8) | src_lo;
    uint32_t res = dst + src;

    if (res > 0xFFFF)
        c->cy = 1;
    else
        c->cy = 0;

    if (((dst & 0xFF) + (src & 0xFF)) > 0xFF)
        c->hf = 1;
    else
        c->hf = 0;

    c->nf = 0;
    *dst_hi = (res >> 8) & 0xFF;
    *dst_lo = res & 0xFF;
}

void cpu_rlca(s_CPU *c)
{
    c->cy = (c->a & 0x80) >> 7;
    c->a = (c->a << 1) | c->cy;
    c->zf = 0;
    c->hf = 0;
    c->nf = 0;
}

void cpu_rrca(s_CPU *c)
{
    c->cy = (c->a & 0x01);
    c->a = (c->a >> 1) | (c->cy << 7);
    c->zf = 0;
    c->hf = 0;
    c->nf = 0;
}

void cpu_rla(s_CPU *c)
{
    uint8_t old_carry = c->cy;
    c->cy = c->a >> 7;
    c->a = (c->a << 1) | old_carry;
    c->zf = 0;
    c->hf = 0;
    c->nf = 0;
}

void cpu_rra(s_CPU *c)
{
    uint8_t old_carry = c->cy;
    c->cy = c->a & 0x01;
    c->a = (c->a >> 1) | (old_carry << 7);
    c->zf = 0;
    c->hf = 0;
    c->nf = 0;
}

void cpu_jr(s_CPU *c)
{
    int8_t value = (int8_t)bus_read(c->pc++);
    c->pc += value;
}

void cpu_cpl(s_CPU *c)
{
    c->a = ~c->a;
    c->nf = 1;
    c->hf = 1;
}

void cpu_scf(s_CPU *c)
{
    c->cy = 1;
    c->nf = 0;
    c->hf = 0;    
}

void cpu_ccf(s_CPU *c)
{
    c->cy = ~c->cy;
    c->nf = 0;
    c->hf = 0;    
}

void cpu_daa(s_CPU *c)
{
    bool flagC = c->cy;
    if (c->nf == 0)
    {
        if ((c->h == 1) || ((c->a & 0x0F) > 0x09))
            c->a += 0x06;
        if ((c->cy == 1) || (c->a > 0x99))
        {
            c->a += 0x60;
            flagC = 1;
        }
    }
    else
    {
        if (c->h == 1)
            c->a -= 0x06;
        if (c->cy == 1)
            c->a -= 0x60;
    }
    c->zf = c->a == 0;
    c->hf = 0;
    c->cy = flagC;
}

void cpu_add(s_CPU *c, uint8_t value)
{
    uint16_t res = c->a + value;
    
    c->zf = (res & 0xFF) == 0;
    c->hf = ((c->a & 0x0F) + (value & 0x0F)) > 0x0F;
    c->cy = res > 0xFF;
    c->nf = 0;
    c->a = res & 0xFF;
}

void cpu_adc(s_CPU *c, uint8_t value)
{
    uint16_t res = c->a + value + c->cy;
    
    c->zf = (res & 0xFF) == 0;
    c->hf = ((c->a & 0x0F) + (value & 0x0F) + c->cy) > 0x0F;
    c->cy = res > 0xFF;
    c->nf = 0;
    c->a = res & 0xFF;
}

void cpu_sub(s_CPU *c, uint8_t value)
{
    uint16_t res = c->a - value;

    c->hf = (((int8_t)c->a & 0xF) - ((int8_t)value & 0xF)) < 0;
    c->cy = ((int8_t)c->a - (int8_t)value) < 0;
    c->zf = (res & 0xFF) == 0;
    c->nf = 1;

    c->a = res & 0xFF;
}

void cpu_sbc(s_CPU *c, uint8_t value)
{
    uint16_t res = c->a - value - c->cy;

    c->hf = (((int8_t)c->a & 0xF) - ((int8_t)value & 0xF) - c->cy) < 0;
    c->cy = ((int8_t)c->a - (int8_t)value - c->cy) < 0;
    c->zf = (res & 0xFF) == 0;
    c->nf = 1;
}

uint8_t execute(s_CPU *c, uint8_t opcode)
{

    uint8_t lo;
    uint8_t hi;
    uint16_t addr;
    switch(opcode) {
        case 0x00: // NOP
            return 4;
        case 0x01: // LD BC,nn
            cpu_ld(false, 0, &c->c, bus_read(c->pc++));
            cpu_ld(false, 0, &c->b, bus_read(c->pc++));
            return 12;
        case 0x02: // LD (BC),A
            addr = ((c->b << 8) | c->c);
            cpu_ld(true, addr, 0, c->a);
            return 8;
        case 0x03: // INC BC
            cpu_inc16(&c->b, &c->c);
            return 8;
        case 0x04: // INC B
            c->b = cpu_inc(c, c->b);
            return 4;
        case 0x05: // DEC B
            c->b = cpu_dec(c, c->b);
            return 4;
        case 0x06: // LD B,n
            cpu_ld(false, 0, &c->b, bus_read(c->pc++));
            return 8;
        case 0x07: // RLCA
            cpu_rlca(c);
            return 4;
        case 0x08: // LD (nn),SP
            lo = bus_read(c->pc++);
            hi = bus_read(c->pc++);
            addr = (hi << 8) | lo;
            cpu_ld(true, addr, 0, c->sp);
            return 20;
        case 0x09: // ADD HL,BC
            cpu_add16(c, &c->h, &c->l, c->b, c->c);
            return 8;
        case 0x0A: // LD A,(BC)
            cpu_ld(false, 0, &c->a, bus_read((c->b << 8) | c->c));
            return 8;
        case 0x0B: // DEC BC
            cpu_dec16(&c->b, &c->c);
            return 8;
        case 0x0C: // INC C
            c->c = cpu_inc(c, c->c);
            return 4;
        case 0x0D: // DEC C
            c->c = cpu_dec(c, c->c);
            return 4;
        case 0x0E: // LD C,n
            cpu_ld(false, 0, &c->c, bus_read(c->pc++));
            return 8;
        case 0x0F: // RRCA
            cpu_rrca(c);
            return 4;
        case 0x10: // STOP
            break;
        case 0x11: // LD DE,nn
            cpu_ld(false, 0, &c->e, bus_read(c->pc++));
            cpu_ld(false, 0, &c->d, bus_read(c->pc++));
            return 12;
        case 0x12: // LD (DE),A
            addr = ((c->d << 8) | c->e);
            cpu_ld(true, addr, 0, c->a);
            return 8;
        case 0x13: // INC DE
            cpu_inc16(&c->d, &c->e);
            return 8;
        case 0x14: // INC D
            c->d = cpu_inc(c, c->d);
            return 4;
        case 0x15: // DEC D
            c->d = cpu_dec(c, c->d);
            return 4;
        case 0x16: // LD D,n
            cpu_ld(false, 0, &c->d, bus_read(c->pc++));
            return 8;
        case 0x17: // RLA
            cpu_rla(c);
            return 4;
        case 0x18: // JR n
            cpu_jr(c);
            return 12;
        case 0x19: // ADD HL,DE
            cpu_add16(c, &c->h, &c->l, c->d, c->e);
            return 8;
        case 0x1A: // LD A,(DE)
            cpu_ld(false, 0, &c->a, bus_read((c->d << 8) | c->e));
            return 8;
        case 0x1B: // DEC DE
            cpu_dec16(&c->d, &c->e);
            return 8;
        case 0x1C: // INC E
            c->e = cpu_inc(c, c->e);
            return 4;
        case 0x1D: // DEC E
            c->e = cpu_dec(c, c->e);
            return 4;
        case 0x1E: // LD E,n
            cpu_ld(false, 0, &c->e, bus_read(c->pc++));
            return 8;
        case 0x1F: // RRA
            cpu_rra(c);
            return 4;
        case 0x20: // JR NZ,n
            if (!c->zf)
            {
                cpu_jr(c);
                return 12;
            }
            return 8;
        case 0x21: // LD HL,nn
            cpu_ld(false, 0, &c->l, bus_read(c->pc++));
            cpu_ld(false, 0, &c->h, bus_read(c->pc++));
            return 12;
        case 0x22: // LD (HL+),A
            addr = ((c->b << 8) | c->c);
            cpu_ld(true, addr, 0, c->a);
            cpu_inc16(&c->h, &c->l);
            return 8;
        case 0x23: // INC HL
            cpu_inc16(&c->h, &c->l);
            return 8;
        case 0x24: // INC H
            c->h = cpu_inc(c, c->h);
            return 4;
        case 0x25: // DEC H
            c->h = cpu_dec(c, c->h);
            return 4;
        case 0x26: // LD H,n
            cpu_ld(false, 0, &c->h, bus_read(c->pc++));
            return 8;
        case 0x27: // DAA
            cpu_daa(c);
            return 4;
        case 0x28: // JR Z,n
            if (c->zf)
            {
                cpu_jr(c);
                return 12;
            }
            return 8;
        case 0x29: // ADD HL,HL
            cpu_add16(c, &c->h, &c->l, c->h, c->l);
            return 8;
        case 0x2A: // LD A,(HL+)
            cpu_ld(false, 0, &c->a, bus_read((c->h << 8) | c->l));
            cpu_inc16(&c->h, &c->l);
            return 8;
        case 0x2B: // DEC HL
            cpu_dec16(&c->h, &c->l);
            return 8;
        case 0x2C: // INC L
            c->l = cpu_inc(c, c->l);
            return 4;
        case 0x2D: // DEC L
            c->l = cpu_dec(c, c->l);
            return 4;
        case 0x2E: // LD L,n
            cpu_ld(false, 0, &c->l, bus_read(c->pc++));
            return 8;
        case 0x2F: // CPL
            cpu_cpl(c);
            return 4;
        case 0x30: // JR NC,n
            if (!c->cy)
            {
                cpu_jr(c);
                return 12;
            }
            return 8;
        case 0x31: // LD SP,nn
            lo = bus_read(c->pc++);
            hi = bus_read(c->pc++);
            c->sp = (hi << 8) | lo;
            return 12;
        case 0x32: // LD (HL-),A
            addr = ((c->b << 8) | c->c);
            cpu_ld(true, addr, 0, c->a);
            cpu_dec16(&c->h, &c->l);
            return 8;
        case 0x33: // INC SP
            hi = c->sp >> 8;
            lo = c->sp & 0xFF;
            cpu_inc16(&hi, &lo);
            c->sp = (hi << 8) | lo;
            return 8;
        case 0x34: // INC (HL)
            hi = bus_read((c->h << 8) | c->l);
            bus_write((c->h << 8) | c->l, cpu_inc(c, hi));
            return 12;
        case 0x35: // DEC (HL)
            hi = bus_read((c->h << 8) | c->l);
            bus_write((c->h << 8) | c->l, cpu_dec(c, hi));
            return 12;
        case 0x36: // LD (HL),n
            cpu_ld(true, (c->h << 8) | c->l, 0, bus_read(c->pc++));
            return 8;
        case 0x37: // SCF
            cpu_scf(c);
            return 4;
        case 0x38: // JR C,n
            if (c->cy)
            {
                cpu_jr(c);
                return 12;
            }
            return 8;
        case 0x39: // ADD HL,SP
            hi = c->sp >> 8;
            lo = c->sp & 0xFF;
            cpu_add16(c, &c->h, &c->l, hi, lo);
            return 8;
        case 0x3A: // LD A,(HL-)
            cpu_ld(false, 0, &c->a, bus_read((c->h << 8) | c->l));
            cpu_dec16(&c->h, &c->l);
            return 8;
        case 0x3B: // DEC SP
            hi = c->sp >> 8;
            lo = c->sp & 0xFF;
            cpu_dec16(&hi, &lo);
            c->sp = (hi << 8) | lo;
            return 8;
        case 0x3C: // INC A
            c->a = cpu_inc(c, c->a);
            return 4;
        case 0x3D: // DEC A
            c->a = cpu_dec(c, c->a);
            return 4;
        case 0x3E: // LD A,n
            cpu_ld(false, 0, &c->a, bus_read(c->pc++));
            return 8;
        case 0x3F: // CCF
            cpu_ccf(c);
            return 4;
        case 0x40: // LD B,B
            cpu_ld(false, 0, &c->b, c->b);
            return 4;
        case 0x41: // LD B,C
            cpu_ld(false, 0, &c->b, c->c);
            return 4;
        case 0x42: // LD B,D
            cpu_ld(false, 0, &c->b, c->d);
            return 4;
        case 0x43: // LD B,E
            cpu_ld(false, 0, &c->b, c->e);
            return 4;
        case 0x44: // LD B,H
            cpu_ld(false, 0, &c->b, c->h);
            return 4;
        case 0x45: // LD B,L
            cpu_ld(false, 0, &c->b, c->l);
            return 4;
        case 0x46: // LD B,(HL)
            cpu_ld(false, 0, &c->b, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x47: // LD B,A
            cpu_ld(false, 0, &c->b, c->a);
            return 4;
        case 0x48: // LD C,B
            cpu_ld(false, 0, &c->c, c->b);
            return 4;
        case 0x49: // LD C,C
            cpu_ld(false, 0, &c->c, c->c);
            return 4;
        case 0x4A: // LD C,D
            cpu_ld(false, 0, &c->c, c->d);
            return 4;
        case 0x4B: // LD C,E
            cpu_ld(false, 0, &c->c, c->e);
            return 4;
        case 0x4C: // LD C,H
            cpu_ld(false, 0, &c->c, c->h);
            return 4;
        case 0x4D: // LD C,L
            cpu_ld(false, 0, &c->c, c->l);
            return 4;
        case 0x4E: // LD C,(HL)
            cpu_ld(false, 0, &c->c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x4F: // LD C,A
            cpu_ld(false, 0, &c->c, c->a);
            return 4;
        case 0x50: // LD D,B
            cpu_ld(false, 0, &c->d, c->b);
            return 4;
        case 0x51: // LD D,C
            cpu_ld(false, 0, &c->d, c->c);
            return 4;
        case 0x52: // LD D,D
            cpu_ld(false, 0, &c->d, c->d);
            return 4;
        case 0x53: // LD D,E
            cpu_ld(false, 0, &c->d, c->e);
            return 4;
        case 0x54: // LD D,H
            cpu_ld(false, 0, &c->d, c->h);
            return 4;
        case 0x55: // LD D,L
            cpu_ld(false, 0, &c->d, c->l);
            return 4;
        case 0x56: // LD D,(HL)
            cpu_ld(false, 0, &c->d, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x57: // LD D,A
            cpu_ld(false, 0, &c->d, c->a);
            return 4;
        case 0x58: // LD E,B
            cpu_ld(false, 0, &c->e, c->b);
            return 4;
        case 0x59: // LD E,C
            cpu_ld(false, 0, &c->e, c->c);
            return 4;
        case 0x5A: // LD E,D
            cpu_ld(false, 0, &c->e, c->d);
            return 4;
        case 0x5B: // LD E,E
            cpu_ld(false, 0, &c->e, c->e);
            return 4;
        case 0x5C: // LD E,H
            cpu_ld(false, 0, &c->e, c->h);
            return 4;
        case 0x5D: // LD E,L
            cpu_ld(false, 0, &c->e, c->l);
            return 4;
        case 0x5E: // LD E,(HL)
            cpu_ld(false, 0, &c->e, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x5F: // LD E,A
            cpu_ld(false, 0, &c->e, c->a);
            return 4;
        case 0x60: // LD H,B
            cpu_ld(false, 0, &c->h, c->b);
            return 4;
        case 0x61: // LD H,C
            cpu_ld(false, 0, &c->h, c->c);
            return 4;
        case 0x62: // LD H,D
            cpu_ld(false, 0, &c->h, c->d);
            return 4;
        case 0x63: // LD H,E
            cpu_ld(false, 0, &c->h, c->e);
            return 4;
        case 0x64: // LD H,H
            cpu_ld(false, 0, &c->h, c->h);
            return 4;
        case 0x65: // LD H,L
            cpu_ld(false, 0, &c->h, c->l);
            return 4;
        case 0x66: // LD H,(HL)
            cpu_ld(false, 0, &c->h, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x67: // LD H,A
            cpu_ld(false, 0, &c->h, c->a);
            return 4;
        case 0x68: // LD L,B
            cpu_ld(false, 0, &c->l, c->b);
            return 4;
        case 0x69: // LD L,C
            cpu_ld(false, 0, &c->l, c->c);
            return 4;
        case 0x6A: // LD L,D
            cpu_ld(false, 0, &c->l, c->d);
            return 4;
        case 0x6B: // LD L,E
            cpu_ld(false, 0, &c->l, c->e);
            return 4;
        case 0x6C: // LD L,H
            cpu_ld(false, 0, &c->l, c->h);
            return 4;
        case 0x6D: // LD L,L
            cpu_ld(false, 0, &c->l, c->l);
            return 4;
        case 0x6E: // LD L,(HL)
            cpu_ld(false, 0, &c->l, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x6F: // LD L,A
            cpu_ld(false, 0, &c->l, c->a);
            return 4;
        case 0x70: // LD (HL),B
            cpu_ld(true, (c->h << 8) | c->l, 0, c->b);
            return 8;
        case 0x71: // LD (HL),C
            cpu_ld(true, (c->h << 8) | c->l, 0, c->c);
            return 8;
        case 0x72: // LD (HL),D
            cpu_ld(true, (c->h << 8) | c->l, 0, c->d);
            return 8;
        case 0x73: // LD (HL),E
            cpu_ld(true, (c->h << 8) | c->l, 0, c->e);
            return 8;
        case 0x74: // LD (HL),H
            cpu_ld(true, (c->h << 8) | c->l, 0, c->h);
            return 8;
        case 0x75: // LD (HL),L
            cpu_ld(true, (c->h << 8) | c->l, 0, c->l);
            return 8;
        case 0x76: // HALT
            break;
        case 0x77: // LD (HL),A
            cpu_ld(true, (c->h << 8) | c->l, 0, c->a);
            return 8;
        case 0x78: // LD A,B
            cpu_ld(false, 0, &c->a, c->b);
            return 4;
        case 0x79: // LD A,C
            cpu_ld(false, 0, &c->a, c->c);
            return 4;
        case 0x7A: // LD A,D
            cpu_ld(false, 0, &c->a, c->d);
            return 4;
        case 0x7B: // LD A,E
            cpu_ld(false, 0, &c->a, c->e);
            return 4;
        case 0x7C: // LD A,H
            cpu_ld(false, 0, &c->a, c->h);
            return 4;
        case 0x7D: // LD A,L
            cpu_ld(false, 0, &c->a, c->l);
            return 4;
        case 0x7E: // LD A,(HL)
            cpu_ld(false, 0, &c->a, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x7F: // LD A,A
            cpu_ld(false, 0, &c->a, c->a);
            return 4;
        case 0x80: // ADD A,B
            cpu_add(c, c->b);
            return 4;
        case 0x81: // ADD A,C
            cpu_add(c, c->c);
            return 4;
        case 0x82: // ADD A,D
            cpu_add(c, c->d);
            return 4;
        case 0x83: // ADD A,E
            cpu_add(c, c->e);
            return 4;
        case 0x84: // ADD A,H
            cpu_add(c, c->h);
            return 4;
        case 0x85: // ADD A,L
            cpu_add(c, c->l);
            return 4;
        case 0x86: // ADD A,(HL)
            cpu_add(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x87: // ADD A,A
            cpu_add(c, c->a);
            return 4;
        case 0x88: // ADC A,B
            cpu_adc(c, c->b);
            return 4;
        case 0x89: // ADC A,C
            cpu_adc(c, c->c);
            return 4;
        case 0x8A: // ADC A,D
            cpu_adc(c, c->d);
            return 4;
        case 0x8B: // ADC A,E
            cpu_adc(c, c->e);
            return 4;
        case 0x8C: // ADC A,H
            cpu_adc(c, c->h);
            return 4;
        case 0x8D: // ADC A,L
            cpu_adc(c, c->l);
            return 4;
        case 0x8E: // ADC A,(HL)
            cpu_adc(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x8F: // ADC A,A
            cpu_adc(c, c->a);
            return 4;
        case 0x90: // SUB B
            cpu_sub(c, c->b);
            return 4;
        case 0x91: // SUB C
            cpu_sub(c, c->c);
            return 4;
        case 0x92: // SUB D
            cpu_sub(c, c->d);
            return 4;
        case 0x93: // SUB E
            cpu_sub(c, c->e);
            return 4;
        case 0x94: // SUB H
            cpu_sub(c, c->h);
            return 4;
        case 0x95: // SUB L
            cpu_sub(c, c->l);
            return 4;
        case 0x96: // SUB (HL)
            cpu_sub(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x97: // SUB A
            cpu_sub(c, c->a);
            return 4;
        case 0x98: // SBC A,B
            cpu_sbc(c, c->b);
            return 4;
        case 0x99: // SBC A,C
            cpu_sbc(c, c->c);
            return 4;
        case 0x9A: // SBC A,D
            cpu_sbc(c, c->d);
            return 4;
        case 0x9B: // SBC A,E
            cpu_sbc(c, c->e);
            return 4;
        case 0x9C: // SBC A,H
            cpu_sbc(c, c->h);
            return 4;
        case 0x9D: // SBC A,L
            cpu_sbc(c, c->l);
            return 4;
        case 0x9E: // SBC A,(HL)
            cpu_sbc(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0x9F: // SBC A,A
            cpu_sbc(c, c->a);
            return 4;
        case 0xA0: // AND B
            break;
        case 0xA1: // AND C
            break;
        case 0xA2: // AND D
            break;
        case 0xA3: // AND E
            break;
        case 0xA4: // AND H
            break;
        case 0xA5: // AND L
            break;
        case 0xA6: // AND (HL)
            break;
        case 0xA7: // AND A
            break;
        case 0xA8: // XOR B
            break;
        case 0xA9: // XOR C
            break;
        case 0xAA: // XOR D
            break;
        case 0xAB: // XOR E
            break;
        case 0xAC: // XOR H
            break;
        case 0xAD: // XOR L
            break;
        case 0xAE: // XOR (HL)
            break;
        case 0xAF: // XOR A
            break;
        case 0xB0: // OR B
            break;
        case 0xB1: // OR C
            break;
        case 0xB2: // OR D
            break;
        case 0xB3: // OR E
            break;
        case 0xB4: // OR H
            break;
        case 0xB5: // OR L
            break;
        case 0xB6: // OR (HL)
            break;
        case 0xB7: // OR A
            break;
        case 0xB8: // CP B
            break;
        case 0xB9: // CP C
            break;
        case 0xBA: // CP D
            break;
        case 0xBB: // CP E
            break;
        case 0xBC: // CP H
            break;
        case 0xBD: // CP L
            break;
        case 0xBE: // CP (HL)
            break;
        case 0xBF: // CP A
            break;
        case 0xC0: // RET NZ
            break;
        case 0xC1: // POP BC
            break;
        case 0xC2: // JP NZ,nn
            break;
        case 0xC3: // JP nn
            break;
        case 0xC4: // CALL NZ,nn
            break;
        case 0xC5: // PUSH BC
            break;
        case 0xC6: // ADD A,n
            break;
        case 0xC7: // RST 00H
            break;
        case 0xC8: // RET Z
            break;
        case 0xC9: // RET
            break;
        case 0xCA: // JP Z,nn
            break;
        case 0xCB: // CB prefix
            break;
        case 0xCC: // CALL Z,nn
            break;
        case 0xCD: // CALL nn
            break;
        case 0xCE: // ADC A,n
            break;
        case 0xCF: // RST 08H
            break;
        case 0xD0: // RET NC
            break;
        case 0xD1: // POP DE
            break;
        case 0xD2: // JP NC,nn
            break;
        case 0xD3: // Illegal Opcode
            break;
        case 0xD4: // CALL NC,nn
            break;
        case 0xD5: // PUSH DE
            break;
        case 0xD6: // SUB n
            break;
        case 0xD7: // RST 10H
            break;
        case 0xD8: // RET C
            break;
        case 0xD9: // RETI
            break;
        case 0xDA: // JP C,nn
            break;
        case 0xDB: // Illegal Opcode
            break;
        case 0xDC: // CALL C,nn
            break;
        case 0xDD: // Illegal Opcode
            break;
        case 0xDE: // SBC A,n
            break;
        case 0xDF: // RST 18H
            break;
        case 0xE0: // LDH (n),A
            break;
        case 0xE1: // POP HL
            break;
        case 0xE2: // LD (C),A
            break;
        case 0xE3: // Illegal Opcode
            break;
        case 0xE4: // Illegal Opcode
            break;
        case 0xE5: // PUSH HL
            break;
        case 0xE6: // AND n
            break;
        case 0xE7: // RST 20H
            break;
        case 0xE8: // ADD SP,n
            break;
        case 0xE9: // JP (HL)
            break;
        case 0xEA: // LD (nn),A
            break;
        case 0xEB: // Illegal Opcode
            break;
        case 0xEC: // Illegal Opcode
            break;
        case 0xED: // Illegal Opcode
            break;
        case 0xEE: // XOR n
            break;
        case 0xEF: // RST 28H
            break;
        case 0xF0: // LDH A,(n)
            break;
        case 0xF1: // POP AF
            break;
        case 0xF2: // LD A,(C)
            break;
        case 0xF3: // DI
            break;
        case 0xF4: // Illegal Opcode
            break;
        case 0xF5: // PUSH AF
            break;
        case 0xF6: // OR n
            break;
        case 0xF7: // RST 30H
            break;
        case 0xF8: // LD HL,SP+n
            break;
        case 0xF9: // LD SP,HL
            break;
        case 0xFA: // LD A,(nn)
            break;
        case 0xFB: // EI
            break;
        case 0xFC: // Illegal Opcode
            break;
        case 0xFD: // Illegal Opcode
            break;
        case 0xFE: // CP n
            break;
        case 0xFF: // RST 38H
            break;
    }
    return 0;
}

void step_emu(s_CPU *c)
{
    uint8_t opcode = bus_read(c->pc++);

    execute(c, opcode);
}