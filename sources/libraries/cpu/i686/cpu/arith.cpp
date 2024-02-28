#include "cpu.h"

BYTE CpuInternal::cpu_arith8(int op, BYTE dest, BYTE src)
{
    int old_cf;
    switch (op & 7) {
        case 0:
            m_lop2 = src;
            m_lr   = (int8_t)(dest += src);
            m_laux = ADD8;
            break;
        case 1:
            m_lr   = (int8_t)(dest |= src);
            m_laux = BIT;
            break;
        case 2:
            old_cf = cpu_get_cf();
            m_lop1 = dest;
            m_lop2 = src;
            m_lr   = (int8_t)(dest += src + old_cf);
            m_laux = ADC8;
            break;
        case 3:
            old_cf = cpu_get_cf();
            m_lop1 = dest;
            m_lop2 = src;
            m_lr   = (int8_t)(dest -= src + old_cf);
            m_laux = SBB8;
            break;
        case 4:
            m_lr   = (int8_t)(dest &= src);
            m_laux = BIT;
            break;
        case 5:
            m_lop2 = src;
            m_lr   = (int8_t)(dest -= src);
            m_laux = SUB8;
            break;
        case 6:
            m_lr   = (int8_t)(dest ^= src);
            m_laux = BIT;
            break;
    }

    return dest;
}

WORD CpuInternal::cpu_arith16(int op, WORD dest, WORD src)
{
    int old_cf;
    switch (op & 7) {
        case 0:
            m_lop2 = src;
            m_lr   = (int16_t)(dest += src);
            m_laux = ADD16;
            break;
        case 1:
            m_lr   = (int16_t)(dest |= src);
            m_laux = BIT;
            break;
        case 2:
            old_cf = cpu_get_cf();
            m_lop1 = dest;
            m_lop2 = src;
            m_lr   = (int16_t)(dest += src + old_cf);
            m_laux = ADC16;
            break;
        case 3:
            old_cf = cpu_get_cf();
            m_lop1 = dest;
            m_lop2 = src;
            m_lr   = (int16_t)(dest -= src + old_cf);
            m_laux = SBB16;
            break;
        case 4:
            m_lr   = (int16_t)(dest &= src);
            m_laux = BIT;
            break;
        case 5:
            m_lop2 = src;
            m_lr   = (int16_t)(dest -= src);
            m_laux = SUB16;
            break;
        case 6:
            m_lr   = (int16_t)(dest ^= src);
            m_laux = BIT;
            break;
    }

    return dest;
}

DWORD CpuInternal::cpu_arith32(int op, DWORD dest, DWORD src)
{
    int old_cf;
    switch (op & 7) {
        case 0:
            m_lop2 = src;
            m_lr   = dest += src;
            m_laux = ADD32;
            break;
        case 1:
            m_lr   = dest |= src;
            m_laux = BIT;
            break;
        case 2:
            old_cf = cpu_get_cf();
            m_lop1 = dest;
            m_lop2 = src;
            m_lr   = dest += src + old_cf;
            m_laux = ADC32;
            break;
        case 3:
            old_cf = cpu_get_cf();
            m_lop1 = dest;
            m_lop2 = src;
            m_lr   = dest -= src + old_cf;
            m_laux = SBB32;
            break;
        case 4:
            m_lr   = dest &= src;
            m_laux = BIT;
            break;
        case 5:
            m_lop2 = src;
            m_lr   = dest -= src;
            m_laux = SUB32;
            break;
        case 6:
            m_lr   = dest ^= src;
            m_laux = BIT;
            break;
    }

    return dest;
}

BYTE CpuInternal::cpu_shift8(int op, BYTE dest, BYTE src)
{
    BYTE op1, op2, res;
    int     cf;
    if (src != 0) {
        switch (op & 7) {
            case 0:
                op1 = dest;
                op2 = src & 7;
                if (op2) {
                    res = (op1 << op2) | (op1 >> (8 - op2));
                } else
                    res = dest;
                if (src & 31) {
                    cpu_set_cf(res & 1);
                    cpu_set_of((res ^ (res >> 7)) & 1);
                }
                break;
            case 1:
                op1 = dest;
                op2 = src & 7;
                if (op2) {
                    res = (op1 >> op2) | (op1 << (8 - op2));
                } else
                    res = dest;
                if (src & 31) {
                    cpu_set_cf(res >> 7 & 1);
                    cpu_set_of((res ^ (res << 1)) >> 7 & 1);
                }
                break;
            case 2:
                op1 = dest;
                op2 = (src & 31) % 9;
                if (op2) {
                    cf  = cpu_get_cf();
                    res = (op1 << op2) | (cf << (op2 - 1)) | (op1 >> (9 - op2));
                    cf  = (op1 >> (8 - op2)) & 1;
                    cpu_set_cf(cf);
                    cpu_set_of((cf ^ (res >> 7)) & 1);
                } else
                    res = dest;
                break;
            case 3:
                op1 = dest;
                op2 = (src & 31) % 9;
                if (op2) {
                    cf  = cpu_get_cf();
                    res = (op1 >> op2) | (cf << (8 - op2)) | (op1 << (9 - op2));
                    cf  = (op1 >> (op2 - 1)) & 1;
                    cpu_set_cf(cf);
                    cpu_set_of((res ^ (res << 1)) >> 7 & 1);
                } else
                    res = dest;
                break;
            case 4:
            case 6:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src & 31;
                    res = m_lr = (int8_t)(dest << m_lop2);
                    m_laux     = SHL8;
                } else
                    res = dest;
                break;
            case 5:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src;
                    res = m_lr = (int8_t)(dest >> m_lop2);
                    m_laux     = SHR8;
                } else
                    res = dest;
                break;
            case 7:
                src &= 31;
                if (src) {
                    m_lop1 = (int8_t)dest;
                    m_lop2 = src;
                    res = m_lr = (int8_t)((((int8_t)(dest)) >> m_lop2));
                    m_laux     = SAR8;
                } else
                    res = dest;
                break;
        }
        dest = res;
    }

    return dest;
}

WORD CpuInternal::cpu_shift16(int op, WORD dest, WORD src)
{
    WORD op1, op2, res;
    int      cf;
    if (src != 0) {
        switch (op & 7) {
            case 0:
                op1 = dest;
                op2 = src & 15;
                if (op2) {
                    res = (op1 << op2) | (op1 >> (16 - op2));
                } else
                    res = dest;
                if (src & 31) {
                    cpu_set_cf(res & 1);
                    cpu_set_of((res ^ (res >> 15)) & 1);
                }
                break;
            case 1:
                op1 = dest;
                op2 = src & 15;
                if (op2) {
                    res = (op1 >> op2) | (op1 << (16 - op2));
                } else
                    res = dest;
                if (src & 31) {
                    cpu_set_cf(res >> 15 & 1);
                    cpu_set_of((res ^ (res << 1)) >> 15 & 1);
                }
                break;
            case 2:
                op1 = dest;
                op2 = (src & 31) % 17;
                if (op2) {
                    cf  = cpu_get_cf();
                    res = (op1 << op2) | (cf << (op2 - 1)) | (op1 >> (17 - op2));
                    cf  = (op1 >> (16 - op2)) & 1;
                    cpu_set_cf(cf);
                    cpu_set_of((cf ^ (res >> 15)) & 1);
                } else
                    res = dest;
                break;
            case 3:
                op1 = dest;
                op2 = (src & 31) % 17;
                if (op2) {
                    int cf = cpu_get_cf();
                    res    = (op1 >> op2) | (cf << (16 - op2)) | (op1 << (17 - op2));
                    cf     = (op1 >> (op2 - 1)) & 1;
                    cpu_set_cf(cf);
                    cpu_set_of((res ^ (res << 1)) >> 15 & 1);
                } else
                    res = dest;
                break;
            case 4:
            case 6:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src;
                    res = m_lr = (int16_t)(dest << m_lop2);
                    m_laux     = SHL16;
                    break;
                } else
                    res = dest;
                break;
            case 5:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src;
                    res = m_lr = (int16_t)(dest >> m_lop2);
                    m_laux     = SHR16;
                } else
                    res = dest;
                break;
            case 7:
                src &= 31;
                if (src) {
                    m_lop1 = (int16_t)dest;
                    m_lop2 = src;
                    res = m_lr = (int16_t)((((int16_t)(dest)) >> m_lop2));
                    m_laux     = SAR16;
                } else
                    res = dest;
                break;
        }
        dest = res;
    }

    return dest;
}

DWORD CpuInternal::cpu_shift32(int op, DWORD dest, DWORD src)
{
    DWORD op1, op2, res;
    int      cf;
    if (src != 0) {
        switch (op & 7) {
            case 0:
                op1 = dest;
                op2 = src & 31;
                if (op2) {
                    res = (op1 << op2) | (op1 >> (32 - op2));
                } else
                    res = dest;
                if (src & 31) {
                    cpu_set_cf(res & 1);
                    cpu_set_of((res ^ (res >> 31)) & 1);
                }
                break;
            case 1:
                op1 = dest;
                op2 = src & 31;
                if (op2) {
                    res = (op1 >> op2) | (op1 << (32 - op2));
                } else
                    res = dest;
                if (src & 31) {
                    cpu_set_cf(res >> 31 & 1);
                    cpu_set_of((res ^ (res << 1)) >> 31 & 1);
                }
                break;
            case 2:
                op1 = dest;
                op2 = src & 31;
                if (op2) {
                    cf = cpu_get_cf();
                    if (op2 == 1)
                        res = (op1 << 1) | cf;
                    else
                        res = (op1 << op2) | (cf << (op2 - 1)) | (op1 >> (33 - op2));
                    cf = (op1 >> (32 - op2)) & 1;
                    cpu_set_cf(cf);
                    cpu_set_of((cf ^ (res >> 31)) & 1);
                } else
                    res = dest;
                break;
            case 3:
                op1 = dest;
                op2 = src & 31;
                if (op2) {
                    cf = cpu_get_cf();
                    if (op2 == 1)
                        res = (op1 >> 1) | (cf << 31);
                    else
                        res = (op1 >> op2) | (cf << (32 - op2)) | (op1 << (33 - op2));
                    cf = (op1 >> (op2 - 1)) & 1;
                    cpu_set_cf(cf);
                    cpu_set_of((res ^ (res << 1)) >> 31 & 1);
                } else
                    res = dest;
                break;
            case 4:
            case 6:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src;
                    res = m_lr = dest << m_lop2;
                    m_laux     = SHL32;
                } else
                    res = dest;
                break;
            case 5:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src;
                    res = m_lr = dest >> m_lop2;
                    m_laux     = SHR32;
                } else
                    res = dest;
                break;
            case 7:
                src &= 31;
                if (src) {
                    m_lop1 = dest;
                    m_lop2 = src;
                    res = m_lr = ((int32_t)(dest)) >> m_lop2;
                    m_laux     = SAR32;
                } else
                    res = dest;
                break;
        }
        dest = res;
    }

    return dest;
}

int CpuInternal::cpu_muldiv8(int op, DWORD src)
{
    DWORD result = 0, result_mod;
    int8_t   low, high;
    switch (op & 7) {
        case 0 ... 3:
            util_abort();
            break;
        case 4:
            result = src * m_reg8[0];
            m_lop1 = 0;
            m_lop2 = result >> 8;
            break;
        case 5:
            result = ((DWORD)(int8_t)src * (DWORD)(int8_t)m_reg8[0]);
            low    = result;
            high   = result >> 8;
            m_lop1 = low >> 7;
            m_lop2 = high;
            break;
        case 6:
            if (src == 0) {
                    cpu_exception(0, 0);
                    return 1;
            }
            result = m_reg16[0] / src;
            if (result > 0xFF) {
                    cpu_exception(0, 0);
                    return 1;
            }
            result_mod = m_reg16[0] % src;
            m_reg8[0]  = result;
            m_reg8[1]  = result_mod;
            return 0;
        case 7: {
            int16_t temp;
            if (src == 0) {
                    cpu_exception(0, 0);
                    return 1;
                }
            temp = result = (int16_t)m_reg16[0] / (int8_t)src;
            if (temp > 0x7F || temp < -0x80) {
                    cpu_exception(0, 0);
                    return 1;
                }
            result_mod = (int16_t)m_reg16[0] % (int8_t)src;
            m_reg8[0]  = result;
            m_reg8[1]  = result_mod;
            return 0;
        }
    }
    m_lr       = (int8_t)result;
    m_laux     = MUL;
    m_reg16[0] = result;
    return 0;
}

int CpuInternal::cpu_muldiv16(int op, DWORD src)
{
    DWORD result = 0, result_mod, original;
    int16_t  low, high;
    switch (op & 7) {
        case 0 ... 3:
            util_abort();
            break;
        case 4:
            result = src * m_reg16[0];
            m_lop1 = 0;
            m_lop2 = result >> 16;
            break;
        case 5:
            result = (DWORD)(int16_t)src * (DWORD)(int16_t)m_reg16[0];
            low    = result;
            high   = result >> 16;
            m_lop1 = low >> 15;
            m_lop2 = high;
            break;
        case 6:
            if (src == 0){
                    cpu_exception(0, 0);
                    return 1;
                }
            original = m_reg16[4] << 16 | m_reg16[0];
            result   = original / src;
            if (result > 0xFFFF){
                    cpu_exception(0, 0);
                    return 1;
                }
            result_mod = original % src;
            m_reg16[0] = result;
            m_reg16[4] = result_mod;
            return 0;
        case 7: {
            int32_t temp;
            if (src == 0) {
                    cpu_exception(0, 0);
                    return 1;
                }
            original = m_reg16[4] << 16 | m_reg16[0];
            temp = result = (int32_t)original / (int16_t)src;
            if (temp > 0x7FFF || temp < -0x8000) {
                    cpu_exception(0, 0);
                    return 1;
                }
            result_mod = (int32_t)original % (int16_t)src;
            m_reg16[0] = result;
            m_reg16[4] = result_mod;
            return 0;
        }
    }
    m_lr       = (int16_t)result;
    m_laux     = MUL;
    m_reg16[0] = result;
    m_reg16[4] = result >> 16;
    return 0;
}

int CpuInternal::cpu_muldiv32(int op, DWORD src)
{
    LARGE result = 0, result_mod, original;
    int32_t  low, high;
    switch (op & 7) {
        case 0 ... 3:
            util_abort();
            break;
        case 4:
            result = (LARGE)src * m_reg32[0];
            m_lop1 = 0;
            m_lop2 = result >> 32;
            break;
        case 5:
            result = (LARGE)(int32_t)src * (LARGE)(int32_t)m_reg32[0];
            low    = result;
            high   = result >> 32;
            m_lop1 = low >> 31;
            m_lop2 = high;
            break;
        case 6:
            if (src == 0) {
                    cpu_exception(0, 0);
                    return 1;
                }
            original = (LARGE)m_reg32[2] << 32 | m_reg32[0];
            result   = original / src;
            if (result > 0xFFFFFFFF) {
                    cpu_exception(0, 0);
                    return 1;
                }
            result_mod = original % src;
            m_reg32[0] = result;
            m_reg32[2] = result_mod;
            return 0;
        case 7: {
            int64_t temp;
            if (src == 0) {
                    cpu_exception(0, 0);
                    return 1;
                }
            original = (LARGE)m_reg32[2] << 32 | m_reg32[0];
            temp = result = (int64_t)original / (int32_t)src;
            if (temp > 0x7FFFFFFF || temp < -(int64_t)0x80000000) {
                    cpu_exception(0, 0);
                    return 1;
                }
            result_mod = (int64_t)original % (int32_t)src;
            m_reg32[0] = result;
            m_reg32[2] = result_mod;
            return 0;
        }
    }
    m_lr       = result;
    m_laux     = MUL;
    m_reg32[0] = result;
    m_reg32[2] = result >> 32;
    return 0;
}

BYTE CpuInternal::cpu_neg8(BYTE dest)
{
    m_lr   = (int8_t)(dest = -(m_lop2 = dest));
    m_laux = SUB8;
    return dest;
}

WORD CpuInternal::cpu_neg16(WORD dest)
{
    m_lr   = (int16_t)(dest = -(m_lop2 = dest));
    m_laux = SUB16;
    return dest;
}

DWORD CpuInternal::cpu_neg32(DWORD dest)
{
    m_lr = dest = -(m_lop2 = dest);
    m_laux       = SUB32;
    return dest;
}

WORD CpuInternal::cpu_shrd16(WORD dest_ptr, WORD src, int count)
{
    count &= 0x1F;
    if (count) {
        WORD dest = dest_ptr, result;
        if (count < 16)
            result = (dest >> count) | (src << (16 - count));
        else {
            result = (src >> (count - 16)) | (dest << (32 - count));
            dest   = src;
            count -= 16;
        }
        src       = count;
        m_lr      = (int16_t)result;
        m_lop1    = dest;
        m_lop2    = src;
        m_laux    = SHRD16;
        dest_ptr = result;
    }
    return dest_ptr;
}

DWORD CpuInternal::cpu_shrd32(DWORD dest_ptr, DWORD src, int count)
{
    count &= 0x1F;
    if (count) {
        DWORD dest = dest_ptr, result;
        result        = (dest >> count) | (src << (32 - count));
        m_lr          = result;
        m_lop1        = dest;
        m_lop2        = count;
        m_laux        = SHRD32;
        dest_ptr     = result;
    }

    return dest_ptr;
}

WORD CpuInternal::cpu_shld16(WORD dest_ptr, WORD src, int count)
{
    count &= 0x1F;
    if (count) {
        WORD dest = dest_ptr, result;
        if (count < 16)
            result = (dest << count) | (src >> (16 - count));
        else
            result = (src << (count - 16)) | (dest >> (32 - count));
        if (count > 16)
            dest = src;
        src       = count;
        m_lr      = (int16_t)result;
        m_lop1    = dest;
        m_lop2    = src;
        m_laux    = SHLD16;
        dest_ptr = result;
    }

    return dest_ptr;
}

DWORD CpuInternal::cpu_shld32(DWORD dest_ptr, DWORD src, int count)
{
    count &= 0x1F;
    if (count) {
        DWORD dest = dest_ptr, result;
        result        = (dest << count) | (src >> (32 - count));
        src           = count;
        m_lr          = result;
        m_lop1        = dest;
        m_lop2        = src;
        m_laux        = SHLD32;
        dest_ptr     = result;
    }

    return dest_ptr;
}

BYTE CpuInternal::cpu_inc8(BYTE dest_ptr)
{
    int cf = cpu_get_cf();
    m_eflags &= ~0x01;
    m_eflags |= cf;
    m_lr   = (int8_t)(++dest_ptr);
    m_laux = INC8;

    return dest_ptr;
}

WORD CpuInternal::cpu_inc16(WORD dest_ptr)
{
    int cf = cpu_get_cf();
    m_eflags &= ~0x01;
    m_eflags |= cf;
    m_lr   = (int16_t)(++dest_ptr);
    m_laux = INC16;
    return dest_ptr;
}

DWORD CpuInternal::cpu_inc32(DWORD dest_ptr)
{
    int cf = cpu_get_cf();
    m_eflags &= ~0x01;
    m_eflags |= cf;
    m_lr   = ++dest_ptr;
    m_laux = INC32;
    return dest_ptr;
}

BYTE CpuInternal::cpu_dec8(BYTE dest_ptr)
{
    int cf = cpu_get_cf();
    m_eflags &= ~0x01;
    m_eflags |= cf;
    m_lr   = (int8_t)(--dest_ptr);
    m_laux = DEC8;
    return dest_ptr;
}

WORD CpuInternal::cpu_dec16(WORD dest_ptr)
{
    int cf = cpu_get_cf();
    m_eflags &= ~0x01;
    m_eflags |= cf;
    m_lr   = (int16_t)(--dest_ptr);
    m_laux = DEC16;
    return dest_ptr;
}

DWORD CpuInternal::cpu_dec32(DWORD dest_ptr)
{
    int cf = cpu_get_cf();
    m_eflags &= ~0x01;
    m_eflags |= cf;
    m_lr   = --dest_ptr;
    m_laux = DEC32;
    return dest_ptr;
}

BYTE CpuInternal::cpu_not8(BYTE dest_ptr)
{
    return ~dest_ptr;
}

WORD CpuInternal::cpu_not16(WORD dest_ptr)
{
    return ~dest_ptr;
}

DWORD CpuInternal::cpu_not32(DWORD dest_ptr)
{
    return ~dest_ptr;
}

BYTE CpuInternal::cpu_imul8(BYTE op1, BYTE op2)
{
    WORD result = (WORD)(int8_t)op1 * (WORD)(int8_t)op2;
    m_laux          = MUL;
    int8_t high = result >> 8, low = result;
    m_lop1 = low >> 7;
    m_lop2 = high;
    m_lr   = low;
    return (BYTE)result;
}

WORD CpuInternal::cpu_imul16(WORD op1, WORD op2)
{
    DWORD result = (DWORD)(int16_t)op1 * (DWORD)(int16_t)op2;
    m_laux          = MUL;
    int16_t high = result >> 16, low = result;
    m_lop1 = low >> 15;
    m_lop2 = high;
    m_lr   = low;
    return (WORD)result;
}

DWORD CpuInternal::cpu_imul32(DWORD op1, DWORD op2)
{
    LARGE result = (LARGE)(int32_t)op1 * (LARGE)(int32_t)op2;
    m_laux          = MUL;
    int32_t high = result >> 32, low = result;
    m_lop1 = low >> 31;
    m_lop2 = high;
    m_lr   = low;
    return (DWORD)result;
}

BYTE CpuInternal::cpu_cmpxchg8(BYTE op1, BYTE op2)
{
    m_lop2 = op1;
    m_lr   = (int8_t)(m_reg8[0] - m_lop2);
    m_laux = SUB8;
    if (!m_lr)
        op1 = op2;
    else
        m_reg8[0] = m_lop2;
    return op1;
}

WORD CpuInternal::cpu_cmpxchg16(WORD op1, WORD op2)
{
    m_lop2 = op1;
    m_lr   = (int16_t)(m_reg16[0] - m_lop2);
    m_laux = SUB16;
    if (!m_lr)
        op1 = op2;
    else
        m_reg16[0] = m_lop2;
    return op1;
}

DWORD CpuInternal::cpu_cmpxchg32(DWORD op1, DWORD op2)
{
    m_lop2 = op1;
    m_lr   = m_reg32[0] - m_lop2;
    m_laux = SUB32;
    if (!m_lr)
        op1 = op2;
    else
        m_reg32[0] = m_lop2;
    return op1;
}

void CpuInternal::xadd8(BYTE& op1, BYTE& op2)
{
    m_lop2 = op2;
    m_lr   = (int8_t)(op1 + m_lop2);
    m_laux = ADD8;
    op2   = op1;
    op1   = m_lr;
}

void CpuInternal::xadd16(WORD& op1, WORD& op2)
{
    m_lop2 = op2;
    m_lr   = (int16_t)(op1 + m_lop2);
    m_laux = ADD16;
    op2   = op1;
    op1   = m_lr;
}

void CpuInternal::xadd32(DWORD& op1, DWORD& op2)
{
    m_lop2 = op2;
    m_lr   = op1 + m_lop2;
    m_laux = ADD32;
    op2   = op1;
    op1   = m_lr;
}
