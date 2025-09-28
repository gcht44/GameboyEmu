#include "../includes/cpu.h"
#include "../includes/bus.h"
#include "../includes/ram.h"

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

void push(s_CPU *c, uint8_t value)
{
    bus_write(--(c->sp), value);
}

uint8_t pop(s_CPU *c)
{
    return bus_read(c->sp++);
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

void cpu_and(s_CPU *c, uint8_t value)
{
    c->a &= value;
    c->zf = c->a == 0;
    c->nf = 0;
    c->hf = 1;
    c->cy = 0;
}

void cpu_xor(s_CPU *c, uint8_t value)
{
    c->a ^= value;
    c->zf = c->a == 0;
    c->nf = 0;
    c->hf = 0;
    c->cy = 0;
}

void cpu_or(s_CPU *c, uint8_t value)
{
    c->a |= value;
    c->zf = c->a == 0;
    c->nf = 0;
    c->hf = 0;
    c->cy = 0;
}

void cpu_cp(s_CPU *c, uint8_t value)
{
    uint16_t res = c->a - value;

    c->hf = (((int8_t)c->a & 0xF) - ((int8_t)value & 0xF)) < 0;
    c->cy = ((int8_t)c->a - (int8_t)value) < 0;
    c->zf = (res & 0xFF) == 0;
    c->nf = 1;
}

void cpu_ret(s_CPU *c)
{
    uint8_t lo = pop(c);
    uint8_t hi = pop(c);
    c->pc = (hi << 8) | lo;
}

void cpu_call(s_CPU *c)
{
    uint8_t lo = bus_read(c->pc++);
    uint8_t hi = bus_read(c->pc++);
    push(c, c->pc >> 8);
    push(c, c->pc & 0xFF);
    c->pc = (hi << 8) | lo;
}

void cpu_jp(s_CPU *c)
{
    uint8_t lo = bus_read(c->pc++);
    uint8_t hi = bus_read(c->pc++);
    c->pc = (hi << 8) | lo;
}

void cpu_rst(s_CPU *c, uint16_t addr)
{
    push(c, c->pc >> 8);
    push(c, c->pc & 0xFF);
    c->pc = addr;
}

void cb_rlc(s_CPU *c, uint8_t *reg)
{
    c->cy = *reg & 0x80;
    *reg <<= 1;
    *reg |= c->cy;
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_rrc(s_CPU *c, uint8_t *reg)
{
    c->cy = *reg & 0x01;
    *reg >>= 1;
    *reg |= (c->cy << 7);
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_rl(s_CPU *c, uint8_t *reg)
{
    bool old_cy = c->cy;
    c->cy = *reg & 0x80;
    *reg <<= 1;
    *reg |= old_cy;
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_rr(s_CPU *c, uint8_t *reg)
{
    bool old_cy = c->cy;
    c->cy = *reg & 0x01;
    *reg >>= 1;
    *reg |= (old_cy << 7);
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_sla(s_CPU *c, uint8_t *reg)
{
    c->cy = *reg & 0x80;
    *reg <<= 1;
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_sra(s_CPU *c, uint8_t *reg)
{
    c->cy = *reg & 0x01;
    *reg = (int8_t)*reg >> 1;
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_swap(s_CPU *c, uint8_t *reg)
{
    uint8_t tmp = *reg & 0x0F;
    *reg = (*reg >> 4) | (tmp << 4);
    c->hf = 0;
    c->nf = 0;
    c->cy = 0;
    c->zf = *reg == 0;
}

void cb_srl(s_CPU *c, uint8_t *reg)
{
    c->cy = *reg & 0x01;
    *reg >>= 1;
    c->zf = *reg == 0;
    c->hf = 0;
    c->nf = 0;
}

void cb_bit(s_CPU *c, uint8_t reg, uint8_t n)
{
    c->zf = ((reg & (1 << n)) == 0);
    c->nf = 0;
    c->hf = 1;
}

void cb_res(s_CPU *c, uint8_t *reg, uint8_t n)
{
    *reg = *reg ^ (1 << n);
}

void cb_set(s_CPU *c, uint8_t *reg, uint8_t n)
{
    *reg = *reg | (1 << n);
}

uint8_t prefix_cb_exec(s_CPU *c, uint8_t opcode)
{
    uint8_t tmp;
    switch (opcode)
    {
        case 0x00: // RLC B
            cb_rlc(c, &c->b);
            return 8;
        case 0x01: // RLC C
            cb_rlc(c, &c->c);
            return 8;
        case 0x02: // RLC D
            cb_rlc(c, &c->d);
            return 8;
        case 0x03: // RLC E
            cb_rlc(c, &c->e);
            return 8;
        case 0x04: // RLC H
            cb_rlc(c, &c->h);
            return 8;
        case 0x05: // RLC L
            cb_rlc(c, &c->l);
            return 8;
        case 0x06: // RLC (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_rlc(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x07: // RLC A
            cb_rlc(c, &c->a);
            return 8;
        case 0x08: // RRC B
            cb_rrc(c, &c->b);
            return 8;
        case 0x09: // RRC C
            cb_rrc(c, &c->c);
            return 8;
        case 0x0A: // RRC D
            cb_rrc(c, &c->d);
            return 8;
        case 0x0B: // RRC E
            cb_rrc(c, &c->e);
            return 8;
        case 0x0C: // RRC H
            cb_rrc(c, &c->h);
            return 8;
        case 0x0D: // RRC L
            cb_rrc(c, &c->l);
            return 8;
        case 0x0E: // RRC (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_rrc(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x0F: // RRC A
            cb_rrc(c, &c->a);
            return 8;
        case 0x10: // RL B
            cb_rl(c, &c->b);
            return 8;
        case 0x11: // RL C
            cb_rl(c, &c->c);
            return 8;
        case 0x12: // RL D
            cb_rl(c, &c->d);
            return 8;
        case 0x13: // RL E
            cb_rl(c, &c->e);
            return 8;
        case 0x14: // RL H
            cb_rl(c, &c->h);
            return 8;
        case 0x15: // RL L
            cb_rl(c, &c->l);
            return 8;
        case 0x16: // RL (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_rl(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x17: // RL A
            cb_rl(c, &c->a);
            return 8;
        case 0x18: // RR B
            cb_rr(c, &c->b);
            return 8;
        case 0x19: // RR C
            cb_rr(c, &c->c);
            return 8;
        case 0x1A: // RR D
            cb_rr(c, &c->d);
            return 8;
        case 0x1B: // RR E
            cb_rr(c, &c->e);
            return 8;
        case 0x1C: // RR H
            cb_rr(c, &c->h);
            return 8;
        case 0x1D: // RR L
            cb_rr(c, &c->l);
            return 8;
        case 0x1E: // RR (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_rr(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x1F: // RR A
            cb_rr(c, &c->a);
            return 8;
        case 0x20: // SLA B
            cb_sla(c, &c->b);
            return 8;
        case 0x21: // SLA C
            cb_sla(c, &c->c);
            return 8;
        case 0x22: // SLA D
            cb_sla(c, &c->d);
            return 8;
        case 0x23: // SLA E
            cb_sla(c, &c->e);
            return 8;
        case 0x24: // SLA H
            cb_sla(c, &c->h);
            return 8;
        case 0x25: // SLA L
            cb_sla(c, &c->l);
            return 8;
        case 0x26: // SLA (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_sla(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x27: // SLA A
            cb_sla(c, &c->a);
            return 8;
        case 0x28: // SRA B
            cb_sra(c, &c->b);
            return 8;
        case 0x29: // SRA C
            cb_sra(c, &c->c);
            return 8;
        case 0x2A: // SRA D
            cb_sra(c, &c->d);
            return 8;
        case 0x2B: // SRA E
            cb_sra(c, &c->e);
            return 8;
        case 0x2C: // SRA H
            cb_sra(c, &c->h);
            return 8;
        case 0x2D: // SRA L
            cb_sra(c, &c->l);
            return 8;
        case 0x2E: // SRA (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_sra(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x2F: // SRA A
            cb_sra(c, &c->a);
            return 8;
        case 0x30: // SWAP B
            cb_swap(c, &c->b);
            return 8;
        case 0x31: // SWAP C
            cb_swap(c, &c->c);
            return 8;
        case 0x32: // SWAP D
            cb_swap(c, &c->d);
            return 8;
        case 0x33: // SWAP E
            cb_swap(c, &c->e);
            return 8;
        case 0x34: // SWAP H
            cb_swap(c, &c->h);
            return 8;
        case 0x35: // SWAP L
            cb_swap(c, &c->l);
            return 8;
        case 0x36: // SWAP (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_swap(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x37: // SWAP A
            cb_swap(c, &c->a);
            return 8;
        case 0x38: // SRL B
            cb_srl(c, &c->b);
            return 8;
        case 0x39: // SRL C
            cb_srl(c, &c->c);
            return 8;
        case 0x3A: // SRL D
            cb_srl(c, &c->d);
            return 8;
        case 0x3B: // SRL E
            cb_srl(c, &c->e);
            return 8;
        case 0x3C: // SRL H
            cb_srl(c, &c->h);
            return 8;
        case 0x3D: // SRL L
            cb_srl(c, &c->l);
            return 8;
        case 0x3E: // SRL (HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_srl(c, &tmp);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x3F: // SRL A
            cb_srl(c, &c->a);
            return 8;
        case 0x40: // BIT 0,B
            cb_bit(c, c->b, 0);
            return 8;
        case 0x41: // BIT 0,C
            cb_bit(c, c->c, 0);
            return 8;
        case 0x42: // BIT 0,D
            cb_bit(c, c->d, 0);
            return 8;
        case 0x43: // BIT 0,E
            cb_bit(c, c->e, 0);
            return 8;
        case 0x44: // BIT 0,H
            cb_bit(c, c->h, 0);
            return 8;
        case 0x45: // BIT 0,L
            cb_bit(c, c->l, 0);
            return 8;
        case 0x46: // BIT 0,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 0);
            return 16;
        case 0x47: // BIT 0,A
            cb_bit(c, c->a, 0);
            return 8;
        case 0x48: // BIT 1,B
            cb_bit(c, c->b, 1);
            return 8;
        case 0x49: // BIT 1,C
            cb_bit(c, c->c, 1);
            return 8;
        case 0x4A: // BIT 1,D
            cb_bit(c, c->d, 1);
            return 8;
        case 0x4B: // BIT 1,E
            cb_bit(c, c->e, 1);
            return 8;
        case 0x4C: // BIT 1,H
            cb_bit(c, c->h, 1);
            return 8;
        case 0x4D: // BIT 1,L
            cb_bit(c, c->l, 1);
            return 8;
        case 0x4E: // BIT 1,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 1);
            return 16;
        case 0x4F: // BIT 1,A
            cb_bit(c, c->a, 1);
            return 8;
        case 0x50: // BIT 2,B
            cb_bit(c, c->b, 2);
            return 8;
        case 0x51: // BIT 2,C
            cb_bit(c, c->c, 2);
            return 8;
        case 0x52: // BIT 2,D
            cb_bit(c, c->d, 2);
            return 8;
        case 0x53: // BIT 2,E
            cb_bit(c, c->e, 2);
            return 8;
        case 0x54: // BIT 2,H
            cb_bit(c, c->h, 2);
            return 8;
        case 0x55: // BIT 2,L
            cb_bit(c, c->l, 2);
            return 8;
        case 0x56: // BIT 2,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 2);
            return 16;
        case 0x57: // BIT 2,A
            cb_bit(c, c->a, 2);
            return 8;
        case 0x58: // BIT 3,B
            cb_bit(c, c->b, 3);
            return 8;
        case 0x59: // BIT 3,C
            cb_bit(c, c->c, 3);
            return 8;
        case 0x5A: // BIT 3,D
            cb_bit(c, c->d, 3);
            return 8;
        case 0x5B: // BIT 3,E
            cb_bit(c, c->e, 3);
            return 8;
        case 0x5C: // BIT 3,H
            cb_bit(c, c->h, 3);
            return 8;
        case 0x5D: // BIT 3,L
            cb_bit(c, c->l, 3);
            return 8;
        case 0x5E: // BIT 3,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 3);
            return 16;
        case 0x5F: // BIT 3,A
            cb_bit(c, c->a, 3);
            return 8;
        case 0x60: // BIT 4,B
            cb_bit(c, c->b, 4);
            return 8;
        case 0x61: // BIT 4,C
            cb_bit(c, c->c, 4);
            return 8;
        case 0x62: // BIT 4,D
            cb_bit(c, c->d, 4);
            return 8;
        case 0x63: // BIT 4,E
            cb_bit(c, c->e, 4);
            return 8;
        case 0x64: // BIT 4,H
            cb_bit(c, c->h, 4);
            return 8;
        case 0x65: // BIT 4,L
            cb_bit(c, c->l, 4);
            return 8;
        case 0x66: // BIT 4,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 4);
            return 16;
        case 0x67: // BIT 4,A
            cb_bit(c, c->a, 4);
            return 8;
        case 0x68: // BIT 5,B
            cb_bit(c, c->b, 5);
            return 8;
        case 0x69: // BIT 5,C
            cb_bit(c, c->c, 5);
            return 8;
        case 0x6A: // BIT 5,D
            cb_bit(c, c->d, 5);
            return 8;
        case 0x6B: // BIT 5,E
            cb_bit(c, c->e, 5);
            return 8;
        case 0x6C: // BIT 5,H
            cb_bit(c, c->h, 5);
            return 8;
        case 0x6D: // BIT 5,L
            cb_bit(c, c->l, 5);
            return 8;
        case 0x6E: // BIT 5,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 5);
            return 16;
        case 0x6F: // BIT 5,A
            cb_bit(c, c->a, 5);
            return 8;
        case 0x70: // BIT 6,B
            cb_bit(c, c->b, 6);
            return 8;
        case 0x71: // BIT 6,C
            cb_bit(c, c->c, 6);
            return 8;
        case 0x72: // BIT 6,D
            cb_bit(c, c->d, 6);
            return 8;
        case 0x73: // BIT 6,E
            cb_bit(c, c->e, 6);
            return 8;
        case 0x74: // BIT 6,H
            cb_bit(c, c->h, 6);
            return 8;
        case 0x75: // BIT 6,L
            cb_bit(c, c->l, 6);
            return 8;
        case 0x76: // BIT 6,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 6);
            return 16;
        case 0x77: // BIT 6,A
            cb_bit(c, c->a, 6);
            return 8;
        case 0x78: // BIT 7,B
            cb_bit(c, c->b, 7);
            return 8;
        case 0x79: // BIT 7,C
            cb_bit(c, c->c, 7);
            return 8;
        case 0x7A: // BIT 7,D
            cb_bit(c, c->d, 7);
            return 8;
        case 0x7B: // BIT 7,E
            cb_bit(c, c->e, 7);
            return 8;
        case 0x7C: // BIT 7,H
            cb_bit(c, c->h, 7);
            return 8;
        case 0x7D: // BIT 7,L
            cb_bit(c, c->l, 7);
            return 8;
        case 0x7E: // BIT 7,(HL)
            cb_bit(c, bus_read((c->h << 8) | c->l), 7);
            return 16;
        case 0x7F: // BIT 7,A
            cb_bit(c, c->a, 7);
            return 8;
        case 0x80: // RES 0,B
            cb_res(c, &c->b, 0);
            return 8;
        case 0x81: // RES 0,C
            cb_res(c, &c->c, 0);
            return 8;
        case 0x82: // RES 0,D
            cb_res(c, &c->d, 0);
            return 8;
        case 0x83: // RES 0,E
            cb_res(c, &c->e, 0);
            return 8;
        case 0x84: // RES 0,H
            cb_res(c, &c->h, 0);
            return 8;
        case 0x85: // RES 0,L
            cb_res(c, &c->l, 0);
            return 8;
        case 0x86: // RES 0,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 0);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x87: // RES 0,A
            cb_res(c, &c->a, 0);
            return 8;
        case 0x88: // RES 1,B
            cb_res(c, &c->b, 1);
            return 8;
        case 0x89: // RES 1,C
            cb_res(c, &c->c, 1);
            return 8;
        case 0x8A: // RES 1,D
            cb_res(c, &c->d, 1);
            return 8;
        case 0x8B: // RES 1,E
            cb_res(c, &c->e, 1);
            return 8;
        case 0x8C: // RES 1,H
            cb_res(c, &c->h, 1);
            return 8;
        case 0x8D: // RES 1,L
            cb_res(c, &c->l, 1);
            return 8;
        case 0x8E: // RES 1,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 1);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x8F: // RES 1,A
            cb_res(c, &c->a, 1);
            return 8;
        case 0x90: // RES 2,B
            cb_res(c, &c->b, 2);
            return 8;
        case 0x91: // RES 2,C
            cb_res(c, &c->c, 2);
            return 8;
        case 0x92: // RES 2,D
            cb_res(c, &c->d, 2);
            return 8;
        case 0x93: // RES 2,E
            cb_res(c, &c->e, 2);
            return 8;
        case 0x94: // RES 2,H
            cb_res(c, &c->h, 2);
            return 8;
        case 0x95: // RES 2,L
            cb_res(c, &c->l, 2);
            return 8;
        case 0x96: // RES 2,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 2);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x97: // RES 2,A
            cb_res(c, &c->a, 2);
            return 8;
        case 0x98: // RES 3,B
            cb_res(c, &c->b, 3);
            return 8;
        case 0x99: // RES 3,C
            cb_res(c, &c->c, 3);
            return 8;
        case 0x9A: // RES 3,D
            cb_res(c, &c->d, 3);
            return 8;
        case 0x9B: // RES 3,E
            cb_res(c, &c->e, 3);
            return 8;
        case 0x9C: // RES 3,H
            cb_res(c, &c->h, 3);
            return 8;
        case 0x9D: // RES 3,L
            cb_res(c, &c->l, 3);
            return 8;
        case 0x9E: // RES 3,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 3);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0x9F: // RES 3,A
            cb_res(c, &c->a, 3);
            return 8;
        case 0xA0: // RES 4,B
            cb_res(c, &c->b, 4);
            return 8;
        case 0xA1: // RES 4,C
            cb_res(c, &c->c, 4);
            return 8;
        case 0xA2: // RES 4,D
            cb_res(c, &c->d, 4);
            return 8;
        case 0xA3: // RES 4,E
            cb_res(c, &c->e, 4);
            return 8;
        case 0xA4: // RES 4,H
            cb_res(c, &c->h, 4);
            return 8;
        case 0xA5: // RES 4,L
            cb_res(c, &c->l, 4);
            return 8;
        case 0xA6: // RES 4,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 4);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xA7: // RES 4,A
            cb_res(c, &c->a, 4);
            return 8;
        case 0xA8: // RES 5,B
            cb_res(c, &c->b, 5);
            return 8;
        case 0xA9: // RES 5,C
            cb_res(c, &c->c, 5);
            return 8;
        case 0xAA: // RES 5,D
            cb_res(c, &c->d, 5);
            return 8;
        case 0xAB: // RES 5,E
            cb_res(c, &c->e, 5);
            return 8;
        case 0xAC: // RES 5,H
            cb_res(c, &c->h, 5);
            return 8;
        case 0xAD: // RES 5,L
            cb_res(c, &c->l, 5);
            return 8;
        case 0xAE: // RES 5,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 5);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xAF: // RES 5,A
            cb_res(c, &c->a, 5);
            return 8;
        case 0xB0: // RES 6,B
            cb_res(c, &c->b, 6);
            return 8;
        case 0xB1: // RES 6,C
            cb_res(c, &c->c, 6);
            return 8;
        case 0xB2: // RES 6,D
            cb_res(c, &c->d, 6);
            return 8;
        case 0xB3: // RES 6,E
            cb_res(c, &c->e, 6);
            return 8;
        case 0xB4: // RES 6,H
            cb_res(c, &c->h, 6);
            return 8;
        case 0xB5: // RES 6,L
            cb_res(c, &c->l, 6);
            return 8;
        case 0xB6: // RES 6,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 6);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xB7: // RES 6,A
            cb_res(c, &c->a, 6);
            return 8;
        case 0xB8: // RES 7,B
            cb_res(c, &c->b, 7);
            return 8;
        case 0xB9: // RES 7,C
            cb_res(c, &c->c, 7);
            return 8;
        case 0xBA: // RES 7,D
            cb_res(c, &c->d, 7);
            return 8;
        case 0xBB: // RES 7,E
            cb_res(c, &c->e, 7);
            return 8;
        case 0xBC: // RES 7,H
            cb_res(c, &c->h, 7);
            return 8;
        case 0xBD: // RES 7,L
            cb_res(c, &c->l, 7);
            return 8;
        case 0xBE: // RES 7,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_res(c, &tmp, 7);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xBF: // RES 7,A
            cb_res(c, &c->a, 7);
            return 8;
        case 0xC0: // SET 0,B
            cb_set(c, &c->b, 0);
            return 8;
        case 0xC1: // SET 0,C
            cb_set(c, &c->c, 0);
            return 8;
        case 0xC2: // SET 0,D
            cb_set(c, &c->d, 0);
            return 8;
        case 0xC3: // SET 0,E
            cb_set(c, &c->e, 0);
            return 8;
        case 0xC4: // SET 0,H
            cb_set(c, &c->h, 0);
            return 8;
        case 0xC5: // SET 0,L
            cb_set(c, &c->l, 0);
            return 8;
        case 0xC6: // SET 0,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 0);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xC7: // SET 0,A
            cb_set(c, &c->a, 0);
            return 8;
        case 0xC8: // SET 1,B
            cb_set(c, &c->b, 1);
            return 8;
        case 0xC9: // SET 1,C
            cb_set(c, &c->c, 1);
            return 8;
        case 0xCA: // SET 1,D
            cb_set(c, &c->d, 1);
            return 8;
        case 0xCB: // SET 1,E
            cb_set(c, &c->e, 1);
            return 8;
        case 0xCC: // SET 1,H
            cb_set(c, &c->h, 1);
            return 8;
        case 0xCD: // SET 1,L
            cb_set(c, &c->l, 1);
            return 8;
        case 0xCE: // SET 1,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 1);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xCF: // SET 1,A
            cb_set(c, &c->a, 1);
            return 8;
        case 0xD0: // SET 2,B
            cb_set(c, &c->b, 2);
            return 8;
        case 0xD1: // SET 2,C
            cb_set(c, &c->c, 2);
            return 8;
        case 0xD2: // SET 2,D
            cb_set(c, &c->d, 2);
            return 8;
        case 0xD3: // SET 2,E
            cb_set(c, &c->e, 2);
            return 8;
        case 0xD4: // SET 2,H
            cb_set(c, &c->h, 2);
            return 8;
        case 0xD5: // SET 2,L
            cb_set(c, &c->l, 2);
            return 8;
        case 0xD6: // SET 2,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 2);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xD7: // SET 2,A
            cb_set(c, &c->a, 2);
            return 8;
        case 0xD8: // SET 3,B
            cb_set(c, &c->b, 3);
            return 8;
        case 0xD9: // SET 3,C
            cb_set(c, &c->c, 3);
            return 8;
        case 0xDA: // SET 3,D
            cb_set(c, &c->d, 3);
            return 8;
        case 0xDB: // SET 3,E
            cb_set(c, &c->e, 3);
            return 8;
        case 0xDC: // SET 3,H
            cb_set(c, &c->h, 3);
            return 8;
        case 0xDD: // SET 3,L
            cb_set(c, &c->l, 3);
            return 8;
        case 0xDE: // SET 3,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 3);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xDF: // SET 3,A
            cb_set(c, &c->a, 3);
            return 8;
        case 0xE0: // SET 4,B
            cb_set(c, &c->b, 4);
            return 8;
        case 0xE1: // SET 4,C
            cb_set(c, &c->c, 4);
            return 8;
        case 0xE2: // SET 4,D
            cb_set(c, &c->d, 4);
            return 8;
        case 0xE3: // SET 4,E
            cb_set(c, &c->e, 4);
            return 8;
        case 0xE4: // SET 4,H
            cb_set(c, &c->h, 4);
            return 8;
        case 0xE5: // SET 4,L
            cb_set(c, &c->l, 4);
            return 8;
        case 0xE6: // SET 4,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 4);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xE7: // SET 4,A
            cb_set(c, &c->a, 4);
            return 8;
        case 0xE8: // SET 5,B
            cb_set(c, &c->b, 5);
            return 8;
        case 0xE9: // SET 5,C
            cb_set(c, &c->c, 5);
            return 8;
        case 0xEA: // SET 5,D
            cb_set(c, &c->d, 5);
            return 8;
        case 0xEB: // SET 5,E
            cb_set(c, &c->e, 5);
            return 8;
        case 0xEC: // SET 5,H
            cb_set(c, &c->h, 5);
            return 8;
        case 0xED: // SET 5,L
            cb_set(c, &c->l, 5);
            return 8;
        case 0xEE: // SET 5,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 5);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xEF: // SET 5,A
            cb_set(c, &c->a, 5);
            return 8;
        case 0xF0: // SET 6,B
            cb_set(c, &c->b, 6);
            return 8;
        case 0xF1: // SET 6,C
            cb_set(c, &c->c, 6);
            return 8;
        case 0xF2: // SET 6,D
            cb_set(c, &c->d, 6);
            return 8;
        case 0xF3: // SET 6,E
            cb_set(c, &c->e, 6);
            return 8;
        case 0xF4: // SET 6,H
            cb_set(c, &c->h, 6);
            return 8;
        case 0xF5: // SET 6,L
            cb_set(c, &c->l, 6);
            return 8;
        case 0xF6: // SET 6,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 6);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xF7: // SET 6,A
            cb_set(c, &c->a, 6);
            return 8;
        case 0xF8: // SET 7,B
            cb_set(c, &c->b, 7);
            return 8;
        case 0xF9: // SET 7,C
            cb_set(c, &c->c, 7);
            return 8;
        case 0xFA: // SET 7,D
            cb_set(c, &c->d, 7);
            return 8;
        case 0xFB: // SET 7,E
            cb_set(c, &c->e, 7);
            return 8;
        case 0xFC: // SET 7,H
            cb_set(c, &c->h, 7);
            return 8;
        case 0xFD: // SET 7,L
            cb_set(c, &c->l, 7);
            return 8;
        case 0xFE: // SET 7,(HL)
            tmp = bus_read((c->h << 8) | c->l);
            cb_set(c, &tmp, 7);
            bus_write((c->h << 8) | c->l, tmp);
            return 16;
        case 0xFF: // SET 7,A
            cb_set(c, &c->a, 7);
            return 8;
    }
    return 0;
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
            c->halt = 1;
            return 4;
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
            cpu_and(c, c->b);
            return 4;
        case 0xA1: // AND C
            cpu_and(c, c->c);
            return 4;
        case 0xA2: // AND D
            cpu_and(c, c->d);
            return 4;
        case 0xA3: // AND E
            cpu_and(c, c->e);
            return 4;
        case 0xA4: // AND H
            cpu_and(c, c->h);
            return 4;
        case 0xA5: // AND L
            cpu_and(c, c->l);
            return 4;
        case 0xA6: // AND (HL)
            cpu_and(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0xA7: // AND A
            cpu_and(c, c->a);
            return 4;
        case 0xA8: // XOR B
            cpu_xor(c, c->b);
            return 4;
        case 0xA9: // XOR C
            cpu_xor(c, c->c);
            return 4;
        case 0xAA: // XOR D
            cpu_xor(c, c->d);
            return 4;
        case 0xAB: // XOR E
            cpu_xor(c, c->e);
            return 4;
        case 0xAC: // XOR H
            cpu_xor(c, c->h);
            return 4;
        case 0xAD: // XOR L
            cpu_xor(c, c->l);
            return 4;
        case 0xAE: // XOR (HL)
            cpu_xor(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0xAF: // XOR A
            cpu_xor(c, c->a);
            return 4;
        case 0xB0: // OR B
            cpu_or(c, c->b);
            return 4;
        case 0xB1: // OR C
            cpu_or(c, c->c);
            return 4;
        case 0xB2: // OR D
            cpu_or(c, c->d);
            return 4;
        case 0xB3: // OR E
            cpu_or(c, c->e);
            return 4;
        case 0xB4: // OR H
            cpu_or(c, c->h);
            return 4;
        case 0xB5: // OR L
            cpu_or(c, c->l);
            return 4;
        case 0xB6: // OR (HL)
            cpu_or(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0xB7: // OR A
            cpu_or(c, c->a);
            return 4;
        case 0xB8: // CP B
            cpu_cp(c, c->b);
            return 4;
        case 0xB9: // CP C
            cpu_cp(c, c->c);
            return 4;
        case 0xBA: // CP D
            cpu_cp(c, c->d);
            return 4;
        case 0xBB: // CP E
            cpu_cp(c, c->e);
            return 4;
        case 0xBC: // CP H
            cpu_cp(c, c->h);
            return 4;
        case 0xBD: // CP L
            cpu_cp(c, c->l);
            return 4;
        case 0xBE: // CP (HL)
            cpu_cp(c, bus_read((c->h << 8) | c->l));
            return 8;
        case 0xBF: // CP A
            cpu_cp(c, c->a);
            return 4;
        case 0xC0: // RET NZ
            if (!c->zf)
            {
                cpu_ret(c);
                return 20;
            }
            return 8;
        case 0xC1: // POP BC
            c->c = pop(c);
            c->b = pop(c);
            return 12;
        case 0xC2: // JP NZ,nn
            if (!c->zf)
            {
                cpu_jp(c);
                return 16;
            }
            return 12;
        case 0xC3: // JP nn
            cpu_jp(c);
            return 16;
        case 0xC4: // CALL NZ,nn
            if (!c->zf)
            {
                cpu_call(c);
                return 24;
            }
            return 12;
        case 0xC5: // PUSH BC
            push(c, c->b);
            push(c, c->c);
            return 16;
        case 0xC6: // ADD A,n
            cpu_add(c, bus_read(c->pc++));
            return 8;
        case 0xC7: // RST 00H
            cpu_rst(c, 0x0000);
            return 16;
        case 0xC8: // RET Z
            if (c->zf)
            {
                cpu_ret(c);
                return 20;
            }
            return 8;
        case 0xC9: // RET
            cpu_ret(c);
            return 20;
        case 0xCA: // JP Z,nn
            if (c->zf)
            {
                cpu_jp(c);
                return 16;
            }
            return 12;
        case 0xCB: // CB prefix
            break;
        case 0xCC: // CALL Z,nn
            if (c->zf)
            {
                cpu_call(c);
                return 24;
            }
            return 12;
        case 0xCD: // CALL nn
            cpu_call(c);
            return 24;
        case 0xCE: // ADC A,n
            cpu_adc(c, bus_read(c->pc++));
            return 8;
        case 0xCF: // RST 08H
            cpu_rst(c, 0x0008);
            return 16;
        case 0xD0: // RET NC
            if (!c->cy)
            {
                cpu_ret(c);
                return 20;
            }
            return 8;
        case 0xD1: // POP DE
            c->e = pop(c);
            c->d = pop(c);
            return 12;
        case 0xD2: // JP NC,nn
            if (!c->cy)
            {
                cpu_jp(c);
                return 16;
            }
            return 12;
        case 0xD3: // Illegal Opcode
            break;
        case 0xD4: // CALL NC,nn
            if (!c->cy)
            {
                cpu_call(c);
                return 24;
            }
            return 12;
        case 0xD5: // PUSH DE
            push(c, c->d);
            push(c, c->e);
            return 16;
        case 0xD6: // SUB n
            cpu_sub(c, bus_read(c->pc++));
            return 8;
        case 0xD7: // RST 10H
            cpu_rst(c, 0x0010);
            return 16;
        case 0xD8: // RET C
            if (c->cy)
            {
                cpu_ret(c);
                return 20;
            }
            return 8;
        case 0xD9: // RETI
            cpu_ret(c);
            c->ime = 1;
            return 16;
        case 0xDA: // JP C,nn
            if (c->cy)
            {
                cpu_jp(c);
                return 16;
            }
            return 12;
        case 0xDB: // Illegal Opcode
            break;
        case 0xDC: // CALL C,nn
            if (c->cy)
            {
                cpu_call(c);
                return 24;
            }
            return 12;
        case 0xDD: // Illegal Opcode
            break;
        case 0xDE: // SBC A,n
            cpu_sbc(c, bus_read(c->pc++));
            return 8;
        case 0xDF: // RST 18H
            cpu_rst(c, 0x0018);
            return 16;
        case 0xE0: // LDH (n),A
            lo = bus_read(c->pc++);
            cpu_ld(true, 0xFF00+lo, 0, c->a);
            return 12;
        case 0xE1: // POP HL
            c->l = pop(c);
            c->h = pop(c);
            return 12;
        case 0xE2: // LD (C),A
            cpu_ld(true, 0xFF00 + c->c, 0, c->a);
            return 8;
        case 0xE3: // Illegal Opcode
            break;
        case 0xE4: // Illegal Opcode
            break;
        case 0xE5: // PUSH HL
            push(c, c->h);
            push(c, c->l);
            return 16;
        case 0xE6: // AND n
            cpu_and(c, bus_read(c->pc++));
            return 8;
        case 0xE7: // RST 20H
            cpu_rst(c, 0x0020);
            return 16;
        case 0xE8: // ADD SP,n
            lo = bus_read(c->pc++);
            addr = (int16_t)c->sp + (int16_t)lo;
            c->zf = 0;
            c->nf = 0;
            c->hf = ((((int16_t)c->sp & 0xF) + ((int16_t)lo & 0xF)) > 0xF);
            c->cy = ((((int16_t)c->sp & 0xFF) + ((int16_t)lo & 0xFF)) > 0xFF);
            c->sp = addr;
            return 16;
        case 0xE9: // JP (HL)
            c->pc = (c->h << 8) | c->l;
            return 4;
        case 0xEA: // LD (nn),A
            lo = bus_read(c->pc++);
            hi = bus_read(c->pc++);
            cpu_ld(true, (hi << 8) | lo, 0, c->a);
            return 16;
        case 0xEB: // Illegal Opcode
            break;
        case 0xEC: // Illegal Opcode
            break;
        case 0xED: // Illegal Opcode
            break;
        case 0xEE: // XOR n
            cpu_xor(c, bus_read(c->pc++));
            return 8;
        case 0xEF: // RST 28H
            cpu_rst(c, 0x0028);
            return 16;
        case 0xF0: // LDH A,(n)
            lo = bus_read(c->pc++);
            cpu_ld(false, 0, &c->a, 0xFF00+lo);
            return 12;
        case 0xF1: // POP AF
            lo = pop(c);
            c->zf = (lo & 0x80) >> 7;
            c->nf = (lo & 0x40) >> 6;
            c->hf = (lo & 0x20) >> 5;
            c->cy = (lo & 0x10) >> 4;
            c->a = pop(c);
            return 12;
        case 0xF2: // LD A,(C)
            cpu_ld(false, 0, &c->a, bus_read(0xFF00 + c->c));
            return 8;
        case 0xF3: // DI
            break;
        case 0xF4: // Illegal Opcode
            break;
        case 0xF5: // PUSH AF
            push(c, c->a);
            lo = ((c->zf << 7) | (c->nf << 6) | (c->hf << 5) | (c->cy << 4)) & 0xF0;
            push(c, lo);
            return 16;
        case 0xF6: // OR n
            cpu_or(c, bus_read(c->pc++));
            return 8;
        case 0xF7: // RST 30H
            cpu_rst(c, 0x0030);
            return 16;
        case 0xF8: // LD HL,SP+n
            lo = bus_read(c->pc++);
            addr = (int16_t)c->sp + (int16_t)lo;
            c->zf = 0;
            c->nf = 0;
            c->hf = ((((int16_t)c->sp & 0xF) + ((int16_t)lo & 0xF)) > 0xF);
            c->cy = ((((int16_t)c->sp & 0xFF) + ((int16_t)lo & 0xFF)) > 0xFF);
            cpu_ld(false, 0, &c->l, addr & 0xFF);
            cpu_ld(false, 0, &c->h, addr >> 8);
            return 12;
        case 0xF9: // LD SP,HL
            c->sp = (c->h << 8) | c->l;
            return 8;
        case 0xFA: // LD A,(nn)
            lo = bus_read(c->pc++);
            hi = bus_read(c->pc++);
            cpu_ld(false, 0, &c->a, bus_read((hi << 8) | lo));
            return 16;
        case 0xFB: // EI
            break;
        case 0xFC: // Illegal Opcode
            break;
        case 0xFD: // Illegal Opcode
            break;
        case 0xFE: // CP n
            cpu_cp(c, bus_read(c->pc++));
            return 8;
        case 0xFF: // RST 38H
            cpu_rst(c, 0x0038);
            return 16;
    }
    return 0;
}

void step_emu(s_CPU *c)
{
    uint8_t opcode = bus_read(c->pc++);

    execute(c, opcode);
}