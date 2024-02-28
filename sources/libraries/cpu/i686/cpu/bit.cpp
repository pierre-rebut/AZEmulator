#include "cpu.h"

void CpuInternal::bt16(WORD a, int shift)
{
    cpu_set_cf(a >> (shift & 15) & 1);
}
void CpuInternal::bt32(DWORD a, int shift)
{
    cpu_set_cf(a >> (shift & 31) & 1);
}

WORD CpuInternal::bts16(WORD a, int shift)
{
    shift &= 15;
    cpu_set_cf(a >> shift & 1);
    a |= 1 << shift;
    return a;
}

DWORD CpuInternal::bts32(DWORD a, int shift)
{
    shift &= 31;
    cpu_set_cf(a >> shift & 1);
    a |= 1 << shift;
    return a;
}

WORD CpuInternal::btc16(WORD a, int shift)
{
    shift &= 15;
    cpu_set_cf(a >> shift & 1);
    a ^= 1 << shift;
    return a;
}

DWORD CpuInternal::btc32(DWORD a, int shift)
{
    shift &= 31;
    cpu_set_cf(a >> shift & 1);
    a ^= 1 << shift;
    return a;
}

WORD CpuInternal::btr16(WORD a, int shift)
{
    shift &= 15;
    cpu_set_cf(a >> shift & 1);
    a &= ~(1 << shift);
    return a;
}

DWORD CpuInternal::btr32(DWORD a, int shift)
{
    shift &= 31;
    cpu_set_cf(a >> shift & 1);
    a &= ~(1 << shift);
    return a;
}

WORD CpuInternal::bsf16(WORD src, WORD old)
{
    if (src) {
        cpu_set_zf(0);
        return __builtin_ctz(src & 0xFFFF);
    } else {
        cpu_set_zf(1);
        return old;
    }
}
DWORD CpuInternal::bsf32(DWORD src, DWORD old)
{
    m_laux = BIT;
    if (src) {
        m_lr = 1;
        return __builtin_ctz(src);
    } else {
        m_lr = 0;
        return old;
    }
}
WORD CpuInternal::bsr16(WORD src, WORD old)
{
    if (src) {
        cpu_set_zf(0);
        return __builtin_clz(src & 0xFFFF) ^ 31;
    } else {
        cpu_set_zf(1);
        return old;
    }
}
DWORD CpuInternal::bsr32(DWORD src, DWORD old)
{
    if (src) {
        cpu_set_zf(0);
        return __builtin_clz(src) ^ 31;
    } else {
        cpu_set_zf(1);
        return old;
    }
}
