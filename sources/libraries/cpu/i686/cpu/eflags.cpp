#include "cpu.h"

int CpuInternal::cpu_get_sf(void)
{
    return (m_lr ^ m_laux) >> 31;
}
void CpuInternal::cpu_set_sf(int set)
{
    m_laux = (m_laux & ~0x80000000) | ((m_lr ^ (set << 31)) & 0x80000000);
}
int CpuInternal::cpu_get_pf(void)
{
    DWORD v = (m_lr ^ (m_laux & 0x80)) & 0xFF;
    v ^= v >> 4;
    v &= 0x0F;
    return (0x9669 >> v) & 1;
}
void CpuInternal::cpu_set_pf(int set)
{
    m_laux = (m_laux & ~0x80) | (cpu_get_pf() ^ set) << 7;
}
int CpuInternal::cpu_get_oac(void)
{
    DWORD eflags = (cpu_get_of() * 0x800) | (cpu_get_af() * 0x10) | (cpu_get_cf() * 0x01);
    m_eflags        = (m_eflags & ~(0x800 | 0x10 | 0x01)) | (eflags & (0x800 | 0x10 | 0x01));
    return eflags;
}
int CpuInternal::cpu_get_of(void)
{
    DWORD lop1;
    switch (m_laux & 63) {
        case MUL:
            return m_lop1 != m_lop2;
        case BIT:
        case SAR8 ... SAR32:
            return 0;
        case ADD8:
            lop1 = m_lr - m_lop2;
            return ((lop1 ^ m_lop2 ^ 0xFF) & (m_lop2 ^ m_lr)) >> 7 & 1;
        case ADD16:
            lop1 = m_lr - m_lop2;
            return ((lop1 ^ m_lop2 ^ 0xFFFF) & (m_lop2 ^ m_lr)) >> 15 & 1;
        case ADD32:
            lop1 = m_lr - m_lop2;
            return ((lop1 ^ m_lop2 ^ 0xFFFFFFFF) & (m_lop2 ^ m_lr)) >> 31 & 1;
        case SUB8:
            lop1 = m_lop2 + m_lr;
            return ((lop1 ^ m_lop2) & (lop1 ^ m_lr)) >> 7 & 1;
        case SUB16:
            lop1 = m_lop2 + m_lr;
            return ((lop1 ^ m_lop2) & (lop1 ^ m_lr)) >> 15 & 1;
        case SUB32:
            lop1 = m_lop2 + m_lr;
            return ((lop1 ^ m_lop2) & (lop1 ^ m_lr)) >> 31 & 1;
        case ADC8:
            return ((m_lop1 ^ m_lr) & (m_lop2 ^ m_lr)) >> 7 & 1;
        case ADC16:
            return ((m_lop1 ^ m_lr) & (m_lop2 ^ m_lr)) >> 15 & 1;
        case ADC32:
            return ((m_lop1 ^ m_lr) & (m_lop2 ^ m_lr)) >> 31 & 1;
        case SBB8:
            return ((m_lr ^ m_lop1) & (m_lop2 ^ m_lop1)) >> 7 & 1;
        case SBB16:
            return ((m_lr ^ m_lop1) & (m_lop2 ^ m_lop1)) >> 15 & 1;
        case SBB32:
            return ((m_lr ^ m_lop1) & (m_lop2 ^ m_lop1)) >> 31 & 1;
        case SHL8:
            return ((m_lr >> 7) ^ (m_lop1 >> (8 - m_lop2))) & 1;
        case SHL16:
            return ((m_lr >> 15) ^ (m_lop1 >> (16 - m_lop2))) & 1;
        case SHL32:
            return ((m_lr >> 31) ^ (m_lop1 >> (32 - m_lop2))) & 1;
        case SHR8:
            return (m_lr << 1 ^ m_lr) >> 7 & 1;
        case SHR16:
            return (m_lr << 1 ^ m_lr) >> 15 & 1;
        case SHR32:
            return (m_lr << 1 ^ m_lr) >> 31 & 1;
        case SHLD16:
            return cpu_get_cf() ^ (m_lr >> 15 & 1);
        case SHLD32:
            return cpu_get_cf() ^ (m_lr >> 31 & 1);
        case SHRD16:
            return (m_lr << 1 ^ m_lr) >> 15 & 1;
        case SHRD32:
            return (m_lr << 1 ^ m_lr) >> 31 & 1;
        case INC8:
            return (m_lr & 0xFF) == 0x80;
        case INC16:
            return (m_lr & 0xFFFF) == 0x8000;
        case INC32:
            return m_lr == 0x80000000;
        case DEC8:
            return (m_lr & 0xFF) == 0x7F;
        case DEC16:
            return (m_lr & 0xFFFF) == 0x7FFF;
        case DEC32:
            return m_lr == 0x7FFFFFFF;
        case EFLAGS_FULL_UPDATE:
            return m_eflags >> 11 & 1;
        default:
            util_abort();
    }
}
void CpuInternal::cpu_set_of(int set)
{
    cpu_get_oac();
    m_eflags &= ~0x800;
    m_eflags |= set * 0x800;
    m_laux = (m_laux & ~63) | EFLAGS_FULL_UPDATE;
}
int CpuInternal::cpu_get_af(void)
{
    DWORD lop1;
    switch (m_laux & 63) {
        case BIT:
        case MUL:
        case SHL8 ... SHL32:
        case SHR8 ... SHR32:
        case SHLD16 ... SHLD32:
        case SHRD16 ... SHRD32:
            return 0;
        case SAR8 ... SAR32:
            return 0;
        case ADD8 ... ADD32:
            lop1 = m_lr - m_lop2;
            return (lop1 ^ m_lop2 ^ m_lr) >> 4 & 1;
        case SUB8 ... SUB32:
            lop1 = m_lr + m_lop2;
            return (lop1 ^ m_lop2 ^ m_lr) >> 4 & 1;
        case ADC8 ... ADC32:
        case SBB8 ... SBB32:
            return (m_lop1 ^ m_lop2 ^ m_lr) >> 4 & 1;
        case INC8 ... INC32:
            return (m_lr & 15) == 0;
        case DEC8 ... DEC32:
            return (m_lr & 15) == 15;
        case EFLAGS_FULL_UPDATE:
            return m_eflags >> 4 & 1;
        default:
            util_abort();
    }
}
void CpuInternal::cpu_set_af(int set)
{
    cpu_get_oac();
    m_eflags &= ~0x10;
    m_eflags |= set * 0x10;
    m_laux = (m_laux & ~63) | EFLAGS_FULL_UPDATE;
}
int CpuInternal::cpu_get_cf(void)
{
    DWORD lop1;
    switch (m_laux & 63) {
        case MUL:
            return m_lop1 != m_lop2;
        case ADD8:
            return (m_lr & 0xFF) < (m_lop2 & 0xFF);
        case ADD16:
            return (m_lr & 0xFFFF) < (m_lop2 & 0xFFFF);
        case ADD32:
            return m_lr < m_lop2;
        case SUB8:
            lop1 = m_lop2 + m_lr;
            return m_lop2 > (lop1 & 0xFF);
        case SUB16:
            lop1 = m_lop2 + m_lr;
            return m_lop2 > (lop1 & 0xFFFF);
        case SUB32:
            lop1 = m_lop2 + m_lr;
            return m_lop2 > lop1;
        case ADC8:
            return (m_lop1 ^ ((m_lop1 ^ m_lop2) & (m_lop2 ^ m_lr))) >> 7 & 1;
        case ADC16:
            return (m_lop1 ^ ((m_lop1 ^ m_lop2) & (m_lop2 ^ m_lr))) >> 15 & 1;
        case ADC32:
            return (m_lop1 ^ ((m_lop1 ^ m_lop2) & (m_lop2 ^ m_lr))) >> 31 & 1;
        case SBB8:
            return (m_lr ^ ((m_lr ^ m_lop2) & (m_lop1 ^ m_lop2))) >> 7 & 1;
        case SBB16:
            return (m_lr ^ ((m_lr ^ m_lop2) & (m_lop1 ^ m_lop2))) >> 15 & 1;
        case SBB32:
            return (m_lr ^ ((m_lr ^ m_lop2) & (m_lop1 ^ m_lop2))) >> 31 & 1;
        case SHR8:
        case SAR8:
        case SHR16:
        case SAR16:
        case SHR32:
        case SAR32:
            return m_lop1 >> (m_lop2 - 1) & 1;
        case SHL8:
            return (m_lop1 >> (8 - m_lop2)) & 1;
        case SHL16:
            return (m_lop1 >> (16 - m_lop2)) & 1;
        case SHL32:
            return (m_lop1 >> (32 - m_lop2)) & 1;
        case SHLD16:
            if (m_lop2 <= 16)
                return m_lop1 >> (16 - m_lop2) & 1;
            return m_lop1 >> (32 - m_lop2) & 1;
        case SHLD32:
            return m_lop1 >> (32 - m_lop2) & 1;
        case SHRD16:
        case SHRD32:
            return m_lop1 >> (m_lop2 - 1) & 1;
        case INC8 ... INC32:
        case DEC8 ... DEC32:
        case EFLAGS_FULL_UPDATE:
            return m_eflags & 1;
        case BIT:
            return 0;
        default:
            util_abort();
    }
}
void CpuInternal::cpu_set_cf(int set)
{
    cpu_get_oac();
    m_eflags &= ~0x01;
    m_eflags |= (set * 0x01);
    m_laux = (m_laux & ~63) | EFLAGS_FULL_UPDATE;
}
void CpuInternal::cpu_set_zf(int set)
{
    cpu_set_eflags((cpu_get_eflags() & ~0x40) | (set * 0x40));
}
DWORD CpuInternal::cpu_get_eflags(void)
{
    DWORD eflags = m_eflags & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01);
    eflags |= cpu_get_of() * 0x800;
    eflags |= cpu_get_sf() * 0x80;
    eflags |= (m_lr == 0) * 0x40;
    eflags |= cpu_get_af() * 0x10;
    eflags |= cpu_get_pf() * 0x04;
    eflags |= cpu_get_cf() * 0x01;
    return eflags;
}
int CpuInternal::cpu_cond(int val)
{
    int cond;
    switch (val >> 1 & 7) {
        case 0:
            cond = cpu_get_of();
            break;
        case 1:
            cond = cpu_get_cf();
            break;
        case 2:
            cond = (m_lr == 0);
            break;
        case 3:
            cond = (m_lr == 0) || cpu_get_cf();
            break;
        case 4:
            cond = cpu_get_sf();
            break;
        case 5:
            cond = cpu_get_pf();
            break;
        case 6:
            cond = cpu_get_sf() != cpu_get_of();
            break;
        case 7:
            cond = (m_lr == 0) || (cpu_get_sf() != cpu_get_of());
            break;
    }
    return cond ^ (val & 1);
}
void CpuInternal::cpu_set_eflags(DWORD eflags)
{
    int old_eflags = m_eflags;
    m_eflags = (m_eflags & ~(0x200000 | 0x100000 | 0x80000 | 0x40000 | 0x20000 | 0x10000 | 0x4000 | 0x3000 | 0x800 |
                             0x400 | 0x200 | 0x100 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01)) |
               (eflags & (0x200000 | 0x100000 | 0x80000 | 0x40000 | 0x20000 | 0x10000 | 0x4000 | 0x3000 | 0x800 |
                          0x400 | 0x200 | 0x100 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01));
    m_lr   = !(eflags & 0x40);
    int pf = m_eflags >> 2 & 1, sf = eflags & 0x80;
    m_laux = EFLAGS_FULL_UPDATE | (sf << 24) | ((pf ^ m_lr ^ 1) << 7);
    if ((old_eflags ^ m_eflags) & 0x200)
        do {
            m_cycles += cpu_get_cycles() - m_cycles;
            m_refill_counter = m_cycles_to_run - 1;
            m_cycles_to_run  = 1;
            m_cycle_offset   = 1;
        } while (0);
}
