#include "cpu.h"
#include <math.h>

void CpuInternal::fpu_set_control_word(WORD control_word) {
    control_word |= 0x40;
    m_fpu.control_word = control_word;
    int rounding = m_fpu.control_word >> 10 & 3;
    switch (rounding) {
        case FPU_ROUND_NEAREST:
            m_fpu.status.float_rounding_mode = float_round_nearest_even;
            break;
        case FPU_ROUND_DOWN:
            m_fpu.status.float_rounding_mode = float_round_down;
            break;
        case FPU_ROUND_UP:
            m_fpu.status.float_rounding_mode = float_round_up;
            break;
        case FPU_ROUND_TRUNCATE:
            m_fpu.status.float_rounding_mode = float_round_to_zero;
            break;
    }
    int precision = m_fpu.control_word >> 8 & 3;
    switch (precision) {
        case FPU_PRECISION_24:
            m_fpu.status.float_rounding_precision = 32;
            break;
        case FPU_PRECISION_53:
            m_fpu.status.float_rounding_precision = 64;
            break;
        case FPU_PRECISION_64:
            m_fpu.status.float_rounding_precision = 80;
            break;
    }
    m_fpu.status.float_exception_flags = 0;
    m_fpu.status.float_nan_handling_mode = float_first_operand_nan;
    m_fpu.status.flush_underflow_to_zero = 0;
    m_fpu.status.float_suppress_exception = 0;
    m_fpu.status.float_exception_masks = control_word & 0x3F;
    m_fpu.status.denormals_are_zeros = 0;
}

WORD CpuInternal::fpu_get_status_word(void) {
    return m_fpu.status_word | (m_fpu.ftop << 11);
}

int CpuInternal::is_denormal(WORD exponent, LARGE mantissa) {
    return !(exponent & 0x7FFF) && mantissa;
}

int CpuInternal::is_pseudo_denormal(WORD exponent, LARGE mantissa) {
    return is_denormal(exponent, mantissa) && !(mantissa & 0x8000000000000000ULL);
}

int CpuInternal::is_zero(WORD exponent, LARGE mantissa) {
    return ((exponent & 0x7FFF) | mantissa) == 0;
}

int CpuInternal::is_zero_any_sign(WORD exponent, LARGE mantissa) {
    if (is_zero(exponent, mantissa)) {
        if (exponent & 0x8000)
            return -1;
        else
            return 1;
    } else
        return 0;
}

int CpuInternal::is_negative(WORD exponent, LARGE mantissa) {
    return !is_zero_any_sign(exponent, mantissa) && (exponent & 0x8000) != 0;
}

int CpuInternal::is_invalid(WORD exponent, LARGE mantissa) {
    WORD exponent_without_sign = exponent & 0x7FFF;
    if (exponent_without_sign != 0)
        return (mantissa & 0x8000000000000000ULL) == 0;
    return 0;
}

int CpuInternal::is_infinity(WORD exponent, LARGE mantissa) {
    if (((exponent & 0x7FFF) == 0x7FFF) && (mantissa == 0x8000000000000000ULL))
        return mantissa >> 15 ? -1 : 1;
    return 0;
}

int CpuInternal::is_nan(WORD exponent, LARGE mantissa) {
    if (((exponent & 0x7FFF) == 0x7FFF) && (mantissa != 0x8000000000000000ULL))
        return 1 + ((mantissa & 0x4000000000000000ULL) != 0);
    return 0;
}

int CpuInternal::fpu_get_tag_from_value(floatx80* f) {
    WORD exponent;
    LARGE mantissa;
    {
        exponent = (f)->exp;
        mantissa = (f)->fraction;
    }
    if ((exponent | mantissa) == 0)
        return FPU_TAG_ZERO;
    int x = 0;
    x |= is_infinity(exponent, mantissa);
    x |= is_denormal(exponent, mantissa);
    x |= is_pseudo_denormal(exponent, mantissa);
    x |= is_invalid(exponent, mantissa);
    x |= is_nan(exponent, mantissa);
    if (x)
        return FPU_TAG_SPECIAL;
    return FPU_TAG_VALID;
}

int CpuInternal::fpu_get_tag(int st) {
    return m_fpu.tag_word >> (((st + m_fpu.ftop) & 7) << 1) & 3;
}

void CpuInternal::fpu_set_tag(int st, int v) {
    int shift = ((st + m_fpu.ftop) & 7) << 1;
    m_fpu.tag_word &= ~(3 << shift);
    m_fpu.tag_word |= v << shift;
}

int CpuInternal::fpu_exception_raised(int flags) {
    return (m_fpu.status.float_exception_flags & ~m_fpu.status.float_exception_masks) & flags;
}

void CpuInternal::fpu_stack_fault(void) {
    m_fpu.status.float_exception_flags = (1 << 0) | (1 << 6);
}

void CpuInternal::fpu_commit_sw(void) {
    m_fpu.status_word |= m_partial_sw;
    m_fpu.status_word &= ~m_bits_to_clear | m_partial_sw;
    m_bits_to_clear = 0;
    m_partial_sw = 0;
}

int CpuInternal::fpu_check_exceptions2(int commit_sw) {
    int flags = m_fpu.status.float_exception_flags;
    int unmasked_exceptions = (flags & ~m_fpu.status.float_exception_masks) & 0x3F;
    if (flags & (1 << 5) && (flags & ((1 << 4) | (1 << 3)))) {
        flags &= ~(1 << 5);
        unmasked_exceptions &= ~(1 << 5);
    }
    if (flags & 0x10000) {
        flags &= ~(1 << 9);
    }
    if (flags & ((1 << 0) | (1 << 2) | (1 << 1))) {
        unmasked_exceptions &= (1 << 0) | (1 << 2) | (1 << 1);
        flags &= (1 << 0) | (1 << 2) | (1 << 1) | (1 << 6);
    }
    if (commit_sw)
        m_fpu.status_word |= flags;
    else
        m_partial_sw |= flags;
    if (unmasked_exceptions) {
        m_fpu.status_word |= 0x8080;
        if (unmasked_exceptions & ~(1 << 5))
            return 1;
        return 0;
    }
    return 0;
}

int CpuInternal::fpu_check_exceptions(void) {
    return fpu_check_exceptions2(1);
}

void CpuInternal::fninit(void) {
    fpu_set_control_word(0x37F);
    m_fpu.status_word = 0;
    m_fpu.tag_word = 0xFFFF;
    m_fpu.ftop = 0;
    m_fpu.fpu_data_ptr = 0;
    m_fpu.fpu_data_seg = 0;
    m_fpu.fpu_eip = 0;
    m_fpu.fpu_cs = 0;
    m_fpu.fpu_opcode = 0;
}

int CpuInternal::fpu_nm_check(void) {
    if (m_cr[0] & (4 | 8)) {
            cpu_exception(7, 0);
            return 1;
        }
    return 0;
}

floatx80* CpuInternal::fpu_get_st_ptr(int st) {
    return &m_fpu.st[(m_fpu.ftop + st) & 7];
}

floatx80 CpuInternal::fpu_get_st(int st) {
    return m_fpu.st[(m_fpu.ftop + st) & 7];
}

void CpuInternal::fpu_set_st(int st, floatx80 data) {
    fpu_set_tag(st, fpu_get_tag_from_value(&data));
    m_fpu.st[(m_fpu.ftop + st) & 7] = data;
}

int CpuInternal::fpu_check_stack_overflow(int st) {
    int tag = fpu_get_tag(st);
    if (tag != FPU_TAG_EMPTY) {
        m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (1) << 9;
        fpu_stack_fault();
        return 1;
    }
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
    return 0;
}

int CpuInternal::fpu_check_stack_underflow(int st, int commit_sw) {
    int tag = fpu_get_tag(st);
    if (tag == FPU_TAG_EMPTY) {
        fpu_stack_fault();
        if (commit_sw)
            m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (1) << 9;
        else
            m_partial_sw = 1 << 9;
        return 1;
    }
    if (commit_sw)
        m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
    else
        m_bits_to_clear = 1 << 9;
    return 0;
}

int CpuInternal::fpu_exception_masked(int excep) {
    if (excep == (1 << 6))
        excep = (1 << 0);
    return (m_fpu.control_word & excep);
}

int CpuInternal::fpu_push(floatx80 data) {
    m_fpu.ftop = (m_fpu.ftop - 1) & 7;
    fpu_set_st(0, data);
    return 0;
}

void CpuInternal::fpu_pop() {
    fpu_set_tag(0, FPU_TAG_EMPTY);
    m_fpu.ftop = (m_fpu.ftop + 1) & 7;
}

void CpuInternal::fpu_update_pointers(DWORD opcode) {
    m_fpu.fpu_cs = m_seg[1];
    m_fpu.fpu_eip = (m_phys_eip + m_eip_phys_bias);
    m_fpu.fpu_opcode = opcode;
}

void CpuInternal::fpu_update_pointers2(DWORD opcode, DWORD virtaddr, DWORD seg) {
    m_fpu.fpu_cs = m_seg[1];
    m_fpu.fpu_eip = (m_phys_eip + m_eip_phys_bias);
    m_fpu.fpu_opcode = opcode;
    m_fpu.fpu_data_ptr = virtaddr;
    m_fpu.fpu_data_seg = m_seg[seg];
}

int CpuInternal::write_float32(DWORD linaddr, float32 src) {
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = src, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    return 0;
}

int CpuInternal::write_float64(DWORD linaddr, float64 dest) {
    LARGE x = dest;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = (DWORD) x, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = (DWORD) (x >> 32),
                tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    return 0;
}

int CpuInternal::fpu_check_push(void) {
    if (fpu_check_stack_overflow(-1)) {
        fpu_check_exceptions();
        if (m_fpu.control_word & (1 << 0)) {
            fpu_push(IndefiniteNaN);
        } else
            m_fpu.status_word |= 0x80;
        return 1;
    }
    return 0;
}

int CpuInternal::fpu_store_f80(DWORD linaddr, floatx80* data) {
    WORD exponent;
    LARGE mantissa;
    {
        exponent = (data)->exp;
        mantissa = (data)->fraction;
    }

    int shift = m_tlb_shift_write;
    {
        DWORD addr_ = linaddr, shift_ = shift, data_ = (DWORD) mantissa, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> shift, shift))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 4, shift_ = shift, data_ = (DWORD) (mantissa >> 32), tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> shift, shift))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 8, shift_ = shift, data_ = exponent, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> shift, shift))
                return 1;
        } else {
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    return 0;
}

int CpuInternal::fpu_read_f80(DWORD linaddr, floatx80* data) {
    WORD exponent;
    DWORD low, hi;
    int shift = m_tlb_shift_read;
    {
        DWORD addr_ = linaddr, shift_ = shift, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> shift, shift))
                low = m_read_result;
            else
                return 1;
        } else {
            low = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        DWORD addr_ = linaddr + 4, shift_ = shift, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> shift, shift))
                hi = m_read_result;
            else
                return 1;
        } else {
            hi = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        DWORD addr_ = linaddr + 8, shift_ = shift, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> shift, shift))
                exponent = m_read_result;
            else
                return 1;
        } else {
            exponent = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        (data)->exp = exponent;
        (data)->fraction = (LARGE) low | (LARGE) hi << 32;
    }
    return 0;
}

int CpuInternal::fpu_fcom(floatx80 op1, floatx80 op2, int unordered) {
    int relation = floatx80_compare_internal(op1, op2, unordered, &m_fpu.status);
    if (fpu_check_exceptions())
        return 1;

    int bad = relation == float_relation_unordered;
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (bad | (relation == float_relation_less)) << 8;
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (bad) << 10;
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (bad | (relation == float_relation_equal)) << 14;
    return 0;
}

int CpuInternal::fpu_fcomi(floatx80 op1, floatx80 op2, int unordered) {
    int relation = floatx80_compare_internal(op1, op2, unordered, &m_fpu.status);
    if (fpu_check_exceptions())
        return 1;

    int bad = relation == float_relation_unordered;
    int cf = bad | (relation == float_relation_less);
    int pf = bad;
    int zf = bad | (relation == float_relation_equal);

    cpu_set_cf(cf);
    cpu_set_pf(pf);
    cpu_set_zf(zf);
    return 0;
}

int CpuInternal::fstenv(DWORD linaddr, int code16) {
    for (int i = 0; i < 8; i++) {
        if (fpu_get_tag(i) != FPU_TAG_EMPTY)
            fpu_set_tag(i, fpu_get_tag_from_value(&m_fpu.st[(m_fpu.ftop + i) & 7]));
    }

    int x = m_tlb_shift_write;
    if (!code16) {
        {
            DWORD addr_ = linaddr, shift_ = x, data_ = 0xFFFF0000 | m_fpu.control_word,
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> x, x))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = linaddr + 4, shift_ = x, data_ = 0xFFFF0000 | fpu_get_status_word(),
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> x, x))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = linaddr + 8, shift_ = x, data_ = 0xFFFF0000 | m_fpu.tag_word,
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> x, x))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        if (m_cr[0] & 1) {
            {
                DWORD addr_ = linaddr + 12, shift_ = x, data_ = m_fpu.fpu_eip, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 16, shift_ = x, data_ = m_fpu.fpu_cs | (m_fpu.fpu_opcode << 16),
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 20, shift_ = x, data_ = m_fpu.fpu_data_ptr, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 24, shift_ = x, data_ = 0xFFFF0000 | m_fpu.fpu_data_seg,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
        } else {
            DWORD linear_fpu_eip = m_fpu.fpu_eip + (m_fpu.fpu_cs << 4);
            DWORD linear_fpu_data = m_fpu.fpu_data_ptr + (m_fpu.fpu_data_seg << 4);
            {
                DWORD addr_ = linaddr + 12, shift_ = x, data_ = linear_fpu_eip | 0xFFFF0000,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 16, shift_ = x,
                        data_ = (m_fpu.fpu_opcode & 0x7FF) | (linear_fpu_eip >> 4 & 0x0FFFF000),
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 20, shift_ = x, data_ = linear_fpu_data | 0xFFFF0000,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 24, shift_ = x, data_ = linear_fpu_data >> 4 & 0x0FFFF000,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
        }
    } else {
        {
            DWORD addr_ = linaddr, shift_ = x, data_ = m_fpu.control_word, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> x, x))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = linaddr + 2, shift_ = x, data_ = fpu_get_status_word(), tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> x, x))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = linaddr + 4, shift_ = x, data_ = m_fpu.tag_word, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> x, x))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        if (m_cr[0] & 1) {
            {
                DWORD addr_ = linaddr + 6, shift_ = x, data_ = m_fpu.fpu_eip, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 8, shift_ = x, data_ = m_fpu.fpu_cs, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 10, shift_ = x, data_ = m_fpu.fpu_data_ptr, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 12, shift_ = x, data_ = m_fpu.fpu_data_seg, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
        } else {
            DWORD linear_fpu_eip = m_fpu.fpu_eip + (m_fpu.fpu_cs << 4);
            DWORD linear_fpu_data = m_fpu.fpu_data_ptr + (m_fpu.fpu_data_seg << 4);
            {
                DWORD addr_ = linaddr + 6, shift_ = x, data_ = linear_fpu_eip, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 8, shift_ = x,
                        data_ = (m_fpu.fpu_opcode & 0x7FF) | (linear_fpu_eip >> 4 & 0xF000),
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 10, shift_ = x, data_ = linear_fpu_data, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 12, shift_ = x, data_ = linear_fpu_data >> 4 & 0xF000,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
        }
    }
    return 0;
}

int CpuInternal::fldenv(DWORD linaddr, int code16) {
    DWORD temp32;
    if (!code16) {
        {
            DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp32 = m_read_result;
                else
                    return 1;
            } else {
                temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        fpu_set_control_word(temp32);
        {
            DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.status_word = m_read_result;
                else
                    return 1;
            } else {
                m_fpu.status_word = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        m_fpu.ftop = m_fpu.status_word >> 11 & 7;
        m_fpu.status_word &= ~(7 << 11);
        {
            DWORD addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.tag_word = m_read_result;
                else
                    return 1;
            } else {
                m_fpu.tag_word = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        if (m_cr[0] & 1) {
            {
                DWORD addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 16, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_cs = temp32 & 0xFFFF;
            m_fpu.fpu_opcode = temp32 >> 16 & 0x7FF;
            {
                DWORD addr_ = linaddr + 20, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_ptr = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_data_ptr = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 24, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_seg = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_data_seg = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
        } else {
            m_fpu.fpu_cs = 0;
            m_fpu.fpu_eip = 0;
            {
                DWORD addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 16, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_opcode = temp32 & 0x7FF;
            m_fpu.fpu_eip |= temp32 << 4 & 0xFFFF0000;
            {
                DWORD addr_ = linaddr + 20, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_data_ptr = temp32 & 0xFFFF;
            {
                DWORD addr_ = linaddr + 24, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_eip |= temp32 << 4 & 0xFFFF0000;
        }
    } else {
        {
            DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp32 = m_read_result;
                else
                    return 1;
            } else {
                temp32 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        fpu_set_control_word(temp32);
        {
            DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.status_word = m_read_result;
                else
                    return 1;
            } else {
                m_fpu.status_word = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        m_fpu.ftop = m_fpu.status_word >> 11 & 7;
        m_fpu.status_word &= ~(7 << 11);
        {
            DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.tag_word = m_read_result;
                else
                    return 1;
            } else {
                m_fpu.tag_word = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        if (m_cr[0] & 1) {
            {
                DWORD addr_ = linaddr + 6, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_cs = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 10, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_ptr = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_data_ptr = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_seg = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_data_seg = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
        } else {
            m_fpu.fpu_cs = 0;
            m_fpu.fpu_eip = 0;
            {
                DWORD addr_ = linaddr + 6, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else {
                    m_fpu.fpu_eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_opcode = temp32 & 0x7FF;
            m_fpu.fpu_eip |= temp32 << 4 & 0xF0000;
            {
                DWORD addr_ = linaddr + 10, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_data_ptr = temp32 & 0xFFFF;
            {
                DWORD addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else {
                    temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            m_fpu.fpu_eip |= temp32 << 4 & 0xF0000;
        }
    }
    if (m_fpu.status_word & ~m_fpu.control_word & 0x3F)
        m_fpu.status_word |= 0x8080;
    else
        m_fpu.status_word &= ~0x8080;
    return 0;
}

void CpuInternal::fpu_watchpoint(void) {
}

void CpuInternal::fpu_watchpoint2(void) {
}

int CpuInternal::fpu_reg_op(struct decoded_instruction* i, DWORD flags) {
    DWORD opcode = i->imm32;
    floatx80 temp80;
    if (fpu_nm_check())
        return 1;
    fpu_watchpoint();
    m_fpu.status.float_exception_flags = 0;

    int smaller_opcode = (opcode >> 5 & 0x38) | (opcode >> 3 & 7);
    switch (smaller_opcode) {
        case (0xD8 & 7) << 3 | 0:
        case (0xD8 & 7) << 3 | 1:
        case (0xD8 & 7) << 3 | 4:
        case (0xD8 & 7) << 3 | 5:
        case (0xD8 & 7) << 3 | 6:
        case (0xD8 & 7) << 3 | 7:
        case (0xDC & 7) << 3 | 0:
        case (0xDC & 7) << 3 | 1:
        case (0xDC & 7) << 3 | 4:
        case (0xDC & 7) << 3 | 5:
        case (0xDC & 7) << 3 | 6:
        case (0xDC & 7) << 3 | 7:
        case (0xDE & 7) << 3 | 0:
        case (0xDE & 7) << 3 | 1:
        case (0xDE & 7) << 3 | 4:
        case (0xDE & 7) << 3 | 5:
        case (0xDE & 7) << 3 | 6:
        case (0xDE & 7) << 3 | 7: {
            int st_index = opcode & 7;
            floatx80 dst;
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(st_index, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            switch (smaller_opcode & 7) {
                case 0:
                    dst = floatx80_add(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 1:
                    dst = floatx80_mul(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 4:
                    dst = floatx80_sub(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 5:
                    dst = floatx80_sub(fpu_get_st(st_index), fpu_get_st(0), &m_fpu.status);
                    break;
                case 6:
                    dst = floatx80_div(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 7:
                    dst = floatx80_div(fpu_get_st(st_index), fpu_get_st(0), &m_fpu.status);
                    break;
            }
            if (!fpu_check_exceptions()) {
                if (smaller_opcode & 32) {
                    fpu_set_st(st_index, dst);
                    if (smaller_opcode & 16)
                        fpu_pop();
                } else
                    fpu_set_st(0, dst);
            }
            break;
        }
        case (0xD8 & 7) << 3 | 2:
        case (0xD8 & 7) << 3 | 3:
        case (0xDC & 7) << 3 | 2:
        case (0xDC & 7) << 3 | 3:
        case (0xDE & 7) << 3 | 2: {
            if (fpu_fwait()){
                    fpu_watchpoint2();
                    return 0;
                }
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
            }
            fpu_update_pointers(opcode);
            if (!fpu_fcom(fpu_get_st(0), fpu_get_st(opcode & 7), 0)) {
                if (smaller_opcode & 1 || smaller_opcode == ((0xDE & 7) << 3 | 2))
                    fpu_pop();
            }
            break;
        }
        case (0xD9 & 7) << 3 | 0:
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }

            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(opcode & 7, 1) || fpu_check_push()) {
                    fpu_watchpoint2();
                    return 0;
                }

            temp80 = fpu_get_st(opcode & 7);
            fpu_push(temp80);
            break;
        case (0xD9 & 7) << 3 | 1:
        case (0xDD & 7) << 3 | 1:
        case (0xDF & 7) << 3 | 1:
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            temp80 = fpu_get_st(0);
            fpu_set_st(0, fpu_get_st(opcode & 7));
            fpu_set_st(opcode & 7, temp80);
            break;
        case (0xD9 & 7) << 3 | 2:
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            break;
        case (0xD9 & 7) << 3 | 4:
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            if ((opcode & 7) != 5)
                if (fpu_check_stack_underflow(0, 1)) {
                        fpu_watchpoint2();
                        return 0;
                    }
            temp80 = fpu_get_st(0);
            switch (opcode & 7) {
                case 0:
                    floatx80_chs(&temp80);
                    break;
                case 1:
                    floatx80_abs(&temp80);
                    break;
                case 4:
                    if (fpu_fcom(temp80, Zero, 0)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    return 0;
                case 5: {
                    int unordered = 0;
                    WORD exponent;
                    LARGE mantissa;
                    {
                        exponent = (&temp80)->exp;
                        mantissa = (&temp80)->fraction;
                    }
                    if (fpu_get_tag(0) == FPU_TAG_EMPTY)
                        unordered = 5;
                    else {
                        if (is_invalid(exponent, mantissa))
                            unordered = 0;
                        else if (is_nan(exponent, mantissa))
                            unordered = 1;
                        else if (is_infinity(exponent, mantissa))
                            unordered = 3;
                        else if (is_zero_any_sign(exponent, mantissa))
                            unordered = 4;
                        else if (is_denormal(exponent, mantissa))
                            unordered = 6;
                        else
                            unordered = 2;
                    }
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (unordered & 1) << 8;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (exponent >> 15 & 1) << 9;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (unordered >> 1 & 1) << 10;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (unordered >> 2 & 1) << 14;
                    return 0;
                }
            }
            fpu_set_st(0, temp80);
            break;
        case (0xD9 & 7) << 3 | 5:
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            if (fpu_check_push()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_push(*Constants[opcode & 7]);
            break;
        case (0xD9 & 7) << 3 | 6: {
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            floatx80 res, temp;
            int temp2, old_rounding;
            switch (opcode & 7) {
                case 0:
                    if (fpu_check_stack_underflow(0, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    res = f2xm1(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions())
                        fpu_set_st(0, res);
                    break;
                case 1:
                    if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    old_rounding = m_fpu.status.float_rounding_precision;
                    m_fpu.status.float_rounding_precision = 80;
                    res = fyl2x(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    m_fpu.status.float_rounding_precision = old_rounding;
                    if (!fpu_check_exceptions()) {
                        fpu_set_st(1, res);
                        fpu_pop();
                    }
                    break;
                case 2:
                    if (fpu_check_stack_underflow(0, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    res = fpu_get_st(0);
                    if (!ftan(&res, &m_fpu.status))
                        fpu_set_st(0, res);
                    break;
                case 3:
                    if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    res = fpatan(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        fpu_pop();
                        fpu_set_st(0, res);
                    }
                    break;
                case 4:
                    if (fpu_check_stack_underflow(0, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    if (fpu_check_stack_overflow(-1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    temp = fpu_get_st(0);
                    res = floatx80_extract(&temp, &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        fpu_set_st(0, res);
                        fpu_push(temp);
                    }
                    break;
                case 5: {
                    floatx80 st0 = fpu_get_st(0), st1 = fpu_get_st(1);
                    LARGE quo;
                    temp2 = floatx80_ieee754_remainder(st0, st1, &temp, &quo, &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        if (!(temp2 < 0)) {
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (0) << 8;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (0) << 10;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (0) << 14;
                            if (temp2 > 0) {
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                            } else {
                                if (quo & 1)
                                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (1) << 9;
                                if (quo & 2)
                                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                                if (quo & 4)
                                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                            }
                        }
                        fpu_set_st(0, temp);
                    }
                }
                    break;
                case 6:
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                    m_fpu.ftop = (m_fpu.ftop - 1) & 7;
                    break;
                case 7:
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                    m_fpu.ftop = (m_fpu.ftop + 1) & 7;
                    break;
            }
        }
            break;

        case (0xD9 & 7) << 3 | 7: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            int flags, pop = 0;
            floatx80 dest;
            LARGE quotient;
            switch (opcode & 7) {
                case 0:
                    if (fpu_check_stack_underflow(1, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    flags = floatx80_remainder(fpu_get_st(0), fpu_get_st(1), &dest, &quotient, &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        if (flags < 0) {
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (0) << 8;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (0) << 10;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (0) << 14;
                        } else {
                            if (flags != 0) {
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (0) << 8;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (0) << 14;
                            } else {
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (quotient >> 2 & 1) << 8;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (quotient & 1) << 9;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (0) << 10;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (quotient >> 1 & 1) << 14;
                            }
                        }
                        fpu_set_st(0, dest);
                    }
                    break;
                case 1:
                    if (fpu_check_stack_underflow(1, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    dest = fyl2xp1(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        fpu_pop();
                        fpu_set_st(0, dest);
                    }
                    return 0;
                case 2:
                    dest = floatx80_sqrt(fpu_get_st(0), &m_fpu.status);
                    break;
                case 3: {
                    if (fpu_check_stack_overflow(-1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    floatx80 sinres, cosres;
                    if (fsincos(fpu_get_st(0), &sinres, &cosres, &m_fpu.status) == -1) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                    } else if (!fpu_check_exceptions()) {
                        fpu_set_st(0, sinres);
                        fpu_push(cosres);
                    }
                    return 0;
                }
                case 4:
                    dest = floatx80_round_to_int(fpu_get_st(0), &m_fpu.status);
                    break;
                case 5:
                    if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1)) {
                            fpu_watchpoint2();
                            return 0;
                        }
                    dest = floatx80_scale(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    break;
                case 6:
                    dest = fpu_get_st(0);
                    if (fsin(&dest, &m_fpu.status) == -1) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                        {
                            fpu_watchpoint2();
                            return 0;
                        }
                    }
                    break;
                case 7:
                    dest = fpu_get_st(0);
                    if (fcos(&dest, &m_fpu.status) == -1) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                        {
                            fpu_watchpoint2();
                            return 0;
                        }
                    }
                    break;
            }
            if (!fpu_check_exceptions()) {
                fpu_set_st(0, dest);
                if (pop)
                    fpu_pop();
            }
            break;
        }
        case (0xDA & 7) << 3 | 0:
        case (0xDB & 7) << 3 | 0:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            if (cpu_get_cf() ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDB & 7) << 3 | 1:
        case (0xDA & 7) << 3 | 1:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            if ((m_lr == 0) ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDB & 7) << 3 | 2:
        case (0xDA & 7) << 3 | 2:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            if (((m_lr == 0) || cpu_get_cf()) ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDB & 7) << 3 | 3:
        case (0xDA & 7) << 3 | 3:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            if (cpu_get_pf() ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDA & 7) << 3 | 5:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if ((opcode & 7) == 1) {
                if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1)) {
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                }
                if (fpu_fcom(fpu_get_st(0), fpu_get_st(1), 1)) {
                        fpu_watchpoint2();
                        return 0;
                    }
                if (!fpu_check_exceptions()) {
                    fpu_pop();
                    fpu_pop();
                }
            }
            break;
        case (0xDB & 7) << 3 | 4:
            switch (opcode & 7) {
                case 0 ... 1:
                case 4:
                    return 0;
                case 2:
                    m_fpu.status_word &= ~(0x80FF);
                    break;
                case 3:
                    fninit();
                    break;
                default: {
                        cpu_exception(6, 0);
                        return 1;
                    }
            }
            break;
        case (0xDB & 7) << 3 | 5:
        case (0xDB & 7) << 3 | 6:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            cpu_set_eflags(cpu_get_eflags() & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01));
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                cpu_set_zf(1);
                cpu_set_pf(1);
                cpu_set_cf(1);
               {
                    fpu_watchpoint2();
                    return 0;
                }
            }
            if (fpu_fcomi(fpu_get_st(0), fpu_get_st(opcode & 7), smaller_opcode & 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            break;
        case (0xD9 & 7) << 3 | 3:
        case (0xDD & 7) << 3 | 2:
        case (0xDD & 7) << 3 | 3:
        case (0xDF & 7) << 3 | 2:
        case (0xDF & 7) << 3 | 3: {
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1)) {
                if (fpu_exception_masked((1 << 6)))
                    fpu_pop();
                {
                    fpu_watchpoint2();
                    return 0;
                }
            }
            fpu_set_st(opcode & 7, fpu_get_st(0));
            if (smaller_opcode & 1 || (smaller_opcode & ~1) == ((0xDF & 7) << 3 | 2))
                fpu_pop();
            break;
        }
        case (0xDD & 7) << 3 | 0:
        case (0xDF & 7) << 3 | 0: {
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            fpu_set_tag(opcode & 7, FPU_TAG_EMPTY);
            if (smaller_opcode == ((0xDF & 7) << 3 | 0))
                fpu_pop();
            break;
        }
        case (0xDD & 7) << 3 | 4:
        case (0xDD & 7) << 3 | 5:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
            }
            if (fpu_fcom(fpu_get_st(0), fpu_get_st(opcode & 7), 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            if (!fpu_check_exceptions()) {
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            break;
        case (0xDE & 7) << 3 | 3:
            if (fpu_fwait()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                if (!fpu_check_exceptions()) {
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                }
                {
                    fpu_watchpoint2();
                    return 0;
                }
            }
            if (fpu_fcom(fpu_get_st(0), fpu_get_st(1), 0))
                {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_pop();
            fpu_pop();
            break;
        case (0xDF & 7) << 3 | 4:
            if ((opcode & 7) != 0){
                    cpu_exception(6, 0);
                    return 1;
                }
            m_reg16[0] = fpu_get_status_word();
            break;
        case (0xDF & 7) << 3 | 5:
        case (0xDF & 7) << 3 | 6: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            cpu_set_eflags(cpu_get_eflags() & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01));
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                cpu_set_zf(1);
                cpu_set_pf(1);
                cpu_set_cf(1);
                {
                    fpu_watchpoint2();
                    return 0;
                }
            }
            if (fpu_fcomi(fpu_get_st(0), fpu_get_st(opcode & 7), smaller_opcode & 1)){
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_pop();
            break;
        }
        case (0xDA & 7) << 3 | 4:
        case (0xDA & 7) << 3 | 6:
        case (0xDA & 7) << 3 | 7:
        case (0xDD & 7) << 3 | 6:
        case (0xDD & 7) << 3 | 7:
        case (0xDF & 7) << 3 | 7: {
            int major_opcode = opcode >> 8 | 0xD8;
            (void) (major_opcode);

            {
                cpu_exception(6, 0);
                return 1;
            }
            break;
        }
        default: {
            int major_opcode = opcode >> 8 | 0xD8;
            util_abort();
        }
    }
    fpu_watchpoint2();
    return 0;
}

int CpuInternal::fpu_mem_op(struct decoded_instruction* i, DWORD virtaddr, DWORD seg) {
    DWORD opcode = i->imm32, linaddr = virtaddr + m_seg_base[seg];
    floatx80 temp80;
    float64 temp64;
    float32 temp32;
    if (fpu_nm_check())
        return 1;
    fpu_watchpoint();
    m_fpu.status.float_exception_flags = 0;
    int smaller_opcode = (opcode >> 5 & 0x38) | (opcode >> 3 & 7);
    switch (smaller_opcode) {
        case (0xD8 & 7) << 3 | 0:
        case (0xD8 & 7) << 3 | 1:
        case (0xD8 & 7) << 3 | 2:
        case (0xD8 & 7) << 3 | 3:
        case (0xD8 & 7) << 3 | 4:
        case (0xD8 & 7) << 3 | 5:
        case (0xD8 & 7) << 3 | 6:
        case (0xD8 & 7) << 3 | 7:
        case (0xD9 & 7) << 3 | 0:
        case (0xDA & 7) << 3 | 0:
        case (0xDA & 7) << 3 | 1:
        case (0xDA & 7) << 3 | 2:
        case (0xDA & 7) << 3 | 3:
        case (0xDA & 7) << 3 | 4:
        case (0xDA & 7) << 3 | 5:
        case (0xDA & 7) << 3 | 6:
        case (0xDA & 7) << 3 | 7:
        case (0xDB & 7) << 3 | 0:
        case (0xDC & 7) << 3 | 0:
        case (0xDC & 7) << 3 | 1:
        case (0xDC & 7) << 3 | 2:
        case (0xDC & 7) << 3 | 3:
        case (0xDC & 7) << 3 | 4:
        case (0xDC & 7) << 3 | 5:
        case (0xDC & 7) << 3 | 6:
        case (0xDC & 7) << 3 | 7:
        case (0xDD & 7) << 3 | 0:
        case (0xDE & 7) << 3 | 0:
        case (0xDE & 7) << 3 | 1:
        case (0xDE & 7) << 3 | 2:
        case (0xDE & 7) << 3 | 3:
        case (0xDE & 7) << 3 | 4:
        case (0xDE & 7) << 3 | 5:
        case (0xDE & 7) << 3 | 6:
        case (0xDE & 7) << 3 | 7:
        case (0xDF & 7) << 3 | 0: {
            if (fpu_fwait())
                return 1;
            switch (opcode >> 9 & 3) {
                case 0:{
                        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                temp32 = m_read_result;
                            else
                                return 1;
                        } else {
                            temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    temp80 = float32_to_floatx80(temp32, &m_fpu.status);
                    break;
                case 1:{
                        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                temp32 = m_read_result;
                            else
                                return 1;
                        } else {
                            temp32 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    temp80 = int32_to_floatx80(temp32);
                    break;
                case 2: {
                    DWORD low, hi;
                    LARGE res;
                    {
                        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                low = m_read_result;
                            else
                                return 1;
                        } else {
                            low = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    {
                        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                hi = m_read_result;
                            else
                                return 1;
                        } else {
                            hi = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    res = (LARGE) low | (LARGE) hi << 32;
                    temp80 = float64_to_floatx80(res, &m_fpu.status);
                    break;
                }
                case 3: {
                    {
                        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 1)) {
                            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                temp32 = m_read_result;
                            else
                                return 1;
                        } else {
                            temp32 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    temp80 = int32_to_floatx80((int16_t) temp32);
                    break;
                }
            }
            fpu_update_pointers2(opcode, virtaddr, seg);
            int op = smaller_opcode & 15;
            if ((op & 8) == 0) {
                if (fpu_check_stack_underflow(0, 1)) {
                    if ((smaller_opcode & 14) == 2) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                    }
                    return 0;
                }
            } else {
                if (fpu_check_push()) {
                        fpu_watchpoint2();
                        return 0;
                    }
            }
            floatx80 st0 = fpu_get_st(0);
            switch (op) {
                case 0:
                    st0 = floatx80_add(st0, temp80, &m_fpu.status);
                    break;
                case 1:
                    st0 = floatx80_mul(st0, temp80, &m_fpu.status);
                    break;
                case 2:
                case 3:
                    if (!fpu_fcom(st0, temp80, 0)) {
                        if (smaller_opcode & 1)
                            fpu_pop();
                    }
                    return 0;
                case 4:
                    st0 = floatx80_sub(st0, temp80, &m_fpu.status);
                    break;
                case 5:
                    st0 = floatx80_sub(temp80, st0, &m_fpu.status);
                    break;
                case 6:
                    st0 = floatx80_div(st0, temp80, &m_fpu.status);
                    break;
                case 7:
                    st0 = floatx80_div(temp80, st0, &m_fpu.status);
                    break;
                default:
                    if (!fpu_check_exceptions())
                        fpu_push(temp80);
                    return 0;
            }
            if (!fpu_check_exceptions())
                fpu_set_st(0, st0);
            break;
        }
        case (0xD9 & 7) << 3 | 2:
        case (0xD9 & 7) << 3 | 3: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0)) {
                    fpu_watchpoint2();
                    return 0;
                }
            temp32 = floatx80_to_float32(fpu_get_st(0), &m_fpu.status);
            if (!fpu_check_exceptions2(0)) {
                if (write_float32(linaddr, temp32))
                    return 1;
                fpu_commit_sw();
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            break;
        }
        case (0xD9 & 7) << 3 | 5: {
            WORD cw;
            {
                DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        cw = m_read_result;
                    else
                        return 1;
                } else {
                    cw = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            fpu_set_control_word(cw);
            break;
        }
        case (0xD9 & 7) << 3 | 6: {
            int is16 = (i->flags & (1 << 25));
            if (fstenv(linaddr, is16)) {
                    fpu_watchpoint2();
                    return 0;
                }
            break;
        }
        case (0xD9 & 7) << 3 | 7: {
                DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_fpu.control_word,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            break;
        case (0xDB & 7) << 3 | 1:
        case (0xDB & 7) << 3 | 2:
        case (0xDB & 7) << 3 | 3:
        case (0xDD & 7) << 3 | 1:
        case (0xDF & 7) << 3 | 1:
        case (0xDF & 7) << 3 | 2:
        case (0xDF & 7) << 3 | 3: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0)) {
                    fpu_watchpoint2();
                    return 0;
                }
            switch (smaller_opcode >> 4 & 3) {
                case 1: {
                    DWORD res;
                    if (smaller_opcode & 2)
                        res = floatx80_to_int32(fpu_get_st(0), &m_fpu.status);
                    else
                        res = floatx80_to_int32_round_to_zero(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions2(0)) {
                            DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = res,
                                    tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                    break;
                }
                case 2: {
                    LARGE res;
                    if (smaller_opcode & 2)
                        res = floatx80_to_int64(fpu_get_st(0), &m_fpu.status);
                    else
                        res = floatx80_to_int64_round_to_zero(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions2(0)) {
                        {
                            DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = res,
                                    tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                        {
                            DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = res >> 32,
                                    tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                    }
                    break;
                }
                case 3: {
                    WORD res;
                    if (smaller_opcode & 2)
                        res = floatx80_to_int16(fpu_get_st(0), &m_fpu.status);
                    else
                        res = floatx80_to_int16_round_to_zero(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions2(0))
                        {
                            DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = res,
                                    tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                    break;
                }
            }
            if (!fpu_check_exceptions2(0)) {
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            fpu_commit_sw();
            break;
        }
        case (0xD9 & 7) << 3 | 4: {
            int is16 = (i->flags & (1 << 25));
            if (fldenv(linaddr, is16)) {
                    fpu_watchpoint2();
                    return 0;
                }
            break;
        }
        case (0xDB & 7) << 3 | 5: {
            if (fpu_fwait())
                return 1;
            if (fpu_read_f80(linaddr, &temp80))
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_overflow(-1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_push(temp80);
            break;
        }
        case (0xDB & 7) << 3 | 7: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 1)) {
                    fpu_watchpoint2();
                    return 0;
                }
            if (fpu_store_f80(linaddr, fpu_get_st_ptr(0)))
                return 1;
            fpu_pop();
            break;
        }
        case (0xDD & 7) << 3 | 2:
        case (0xDD & 7) << 3 | 3: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0)) {
                fpu_watchpoint2();
                return 0;
            }
            temp64 = floatx80_to_float64(fpu_get_st(0), &m_fpu.status);
            if (!fpu_check_exceptions2(0)) {
                if (write_float64(linaddr, temp64))
                    return 1;
                fpu_commit_sw();
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            break;
        }
        case (0xDD & 7) << 3 | 4: {
            int is16 = (i->flags & (1 << 25));
            if (fldenv(linaddr, is16)) {
                    fpu_watchpoint2();
                    return 0;
                }
            int offset = 14 << !is16;
            for (int i = 0; i < 8; i++) {
                if (fpu_read_f80(offset + linaddr, &m_fpu.st[(m_fpu.ftop + i) & 7]))
                    return 1;
                offset += 10;
            }
            break;
        }
        case (0xDD & 7) << 3 | 6: {
            int is16 = (i->flags & (1 << 25));
            if (fstenv(linaddr, is16)) {
                    fpu_watchpoint2();
                    return 0;
                }
            int offset = 14 << !is16;
            for (int i = 0; i < 8; i++) {
                if (fpu_store_f80(offset + linaddr, &m_fpu.st[(m_fpu.ftop + i) & 7]))
                    return 1;
                offset += 10;
            }
            fninit();
            break;
        }
        case (0xDD & 7) << 3 | 7: {
                DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = fpu_get_status_word(),
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            break;
        case (0xDF & 7) << 3 | 4: {
            DWORD low, high, higher;
            if (fpu_fwait())
                return 1;
            {
                DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        low = m_read_result;
                    else
                        return 1;
                } else {
                    low = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        high = m_read_result;
                    else
                        return 1;
                } else {
                    high = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        higher = m_read_result;
                    else
                        return 1;
                } else {
                    higher = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            fpu_update_pointers2(opcode, virtaddr, seg);
            LARGE result = 0;
            int sign = higher & 0x8000;
            higher &= 0x7FFF;

            for (int i = 0; i < 4; i++) {
                result *= 100;
                BYTE temp = low & 0xFF;
                result += temp - (6 * (temp >> 4));
                low >>= 8;
            }

            for (int i = 0; i < 4; i++) {
                result *= 100;
                BYTE temp = high & 0xFF;
                result += temp - (6 * (temp >> 4));
                high >>= 8;
            }

            for (int i = 0; i < 2; i++) {
                result *= 100;
                BYTE temp = higher & 0xFF;
                result += temp - (6 * (temp >> 4));
                higher >>= 8;
            }

            temp80 = int64_to_floatx80((LARGE) sign << (64LL - 16LL) | result);
            if (fpu_check_push()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_push(temp80);
            break;
        }
        case (0xDF & 7) << 3 | 5: {
            DWORD hi, low;
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            {
                DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        low = m_read_result;
                    else
                        return 1;
                } else {
                    low = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            {
                DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        hi = m_read_result;
                    else
                        return 1;
                } else {
                    hi = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
            temp80 = int64_to_floatx80((LARGE) low | (LARGE) hi << 32);
            if (fpu_check_push()) {
                    fpu_watchpoint2();
                    return 0;
                }
            fpu_push(temp80);
            break;
        }
        case (0xDF & 7) << 3 | 6: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            floatx80 st0 = fpu_get_st(0);
            LARGE bcd = floatx80_to_int64(st0, &m_fpu.status);
            if (fpu_check_exceptions()) {
                    fpu_watchpoint2();
                    return 0;
                }

            for (int i = 0; i < 9; i++) {
                int result = bcd % 10;
                bcd /= 10;
                result |= (bcd % 10) << 4;
                bcd /= 10;
                {
                    DWORD addr_ = linaddr + i, shift_ = m_tlb_shift_write, data_ = result,
                            tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
            int result = bcd % 10;
            bcd /= 10;
            result |= (bcd % 10) << 4;
            {
                DWORD addr_ = linaddr + 9, shift_ = m_tlb_shift_write, data_ = result | (st0.exp >> 8 & 0x80),
                        tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            fpu_pop();
            break;
        }
        case (0xDF & 7) << 3 | 7: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0)){
                    fpu_watchpoint2();
                    return 0;
                }

            LARGE i64 = floatx80_to_int64(fpu_get_st(0), &m_fpu.status);
            if (fpu_check_exceptions2(0)) {
                    fpu_watchpoint2();
                    return 0;
                }
             {
                DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = (DWORD) i64,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            {
                DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = (DWORD) (i64 >> 32),
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
            fpu_commit_sw();
            fpu_pop();
            break;
        }
        case (0xD9 & 7) << 3 | 1:
        case (0xDB & 7) << 3 | 4:
        case (0xDB & 7) << 3 | 6:
        case (0xDD & 7) << 3 | 5: {
            int major_opcode = opcode >> 8 | 0xD8;
            (void) (major_opcode);
            break;
        }
        default: {
            int major_opcode = opcode >> 8 | 0xD8;
            util_abort();
        }
    }
    fpu_watchpoint2();
    return 0;
}

int CpuInternal::fpu_fxsave(DWORD linaddr) {
    if (linaddr & 15) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }
    if (fpu_nm_check())
        return 1;
    if (cpu_access_verify(linaddr, linaddr + 288 - 1, m_tlb_shift_write))
        return 1;
    {
        DWORD addr_ = linaddr + 0, shift_ = m_tlb_shift_write, data_ = m_fpu.control_word,
                tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_write, data_ = fpu_get_status_word(),
                tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    BYTE tag = 0;
    for (int i = 0; i < 8; i++)
        if ((m_fpu.tag_word >> (i * 2) & 3) != FPU_TAG_EMPTY)
            tag |= 1 << i;
    {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = tag, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 6, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_opcode,
                tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 8, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_eip, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 12, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_cs, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 16, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_data_ptr,
                tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 20, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_data_seg,
                tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 24, shift_ = m_tlb_shift_write, data_ = m_mxcsr, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    {
        DWORD addr_ = linaddr + 28, shift_ = m_tlb_shift_write, data_ = 0xFFFF, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }
    DWORD tempaddr = linaddr + 32;
    for (int i = 0; i < 8; i++) {
        fpu_store_f80(tempaddr, fpu_get_st_ptr(i));
        {
            DWORD addr_ = tempaddr + 10, shift_ = m_tlb_shift_write, data_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = tempaddr + 12, shift_ = m_tlb_shift_write, data_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        tempaddr += 16;
    }
    tempaddr = linaddr + 160;
    for (int i = 0; i < 8; i++) {
        {
            DWORD addr_ = tempaddr, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4)],
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = tempaddr + 4, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4) + 1],
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = tempaddr + 8, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4) + 2],
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = tempaddr + 12, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4) + 3],
                    tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        tempaddr += 16;
    }
    return 0;
}

int CpuInternal::fpu_fxrstor(DWORD linaddr) {
    if (linaddr & 15) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }
    if (fpu_nm_check())
        return 1;
    if (cpu_access_verify(linaddr, linaddr + 288 - 1, m_tlb_shift_read))
        return 1;
    DWORD _mxcsr;
    {
        DWORD addr_ = linaddr + 24, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                _mxcsr = m_read_result;
            else
                return 1;
        } else {
            _mxcsr = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }
    if (_mxcsr & ~0xFFFF) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }

    m_mxcsr = _mxcsr;
    cpu_update_mxcsr();
    DWORD temp = 0;
    {
        DWORD addr_ = linaddr + 0, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                return 1;
        } else {
            temp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }
    fpu_set_control_word(temp);
    {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                return 1;
        } else {
            temp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }

    m_fpu.status_word = temp;
    m_fpu.ftop = m_fpu.status_word >> 11 & 7;
    m_fpu.status_word &= ~(7 << 11);
    BYTE small_tag_word;
    {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                small_tag_word = m_read_result;
            else
                return 1;
        } else {
            small_tag_word = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        DWORD addr_ = linaddr + 6, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_opcode = m_read_result;
            else
                return 1;
        } else {
            m_fpu.fpu_opcode = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }
    m_fpu.fpu_opcode &= 0x7FF;
    {
        DWORD addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_eip = m_read_result;
            else
                return 1;
        } else {
            m_fpu.fpu_eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        DWORD addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_cs = m_read_result;
            else
                return 1;
        } else {
            m_fpu.fpu_cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        DWORD addr_ = linaddr + 16, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_data_ptr = m_read_result;
            else
                return 1;
        } else {
            m_fpu.fpu_data_ptr = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }
    {
        DWORD addr_ = linaddr + 20, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_data_seg = m_read_result;
            else
                return 1;
        } else {
            m_fpu.fpu_data_seg = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }

    DWORD tempaddr = linaddr + 32;
    for (int i = 0; i < 8; i++) {
        if (fpu_read_f80(tempaddr, fpu_get_st_ptr(i)))
            return 1;
        tempaddr += 16;
    }
    tempaddr = linaddr + 160;
    for (int i = 0; i < 8; i++) {
        {
            DWORD addr_ = tempaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4)] = m_read_result;
                else
                    return 1;
            } else {
                m_xmm32[(i * 4)] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        {
            DWORD addr_ = tempaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4) + 1] = m_read_result;
                else
                    return 1;
            } else {
                m_xmm32[(i * 4) + 1] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        {
            DWORD addr_ = tempaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4) + 2] = m_read_result;
                else
                    return 1;
            } else {
                m_xmm32[(i * 4) + 2] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        {
            DWORD addr_ = tempaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4) + 3] = m_read_result;
                else
                    return 1;
            } else {
                m_xmm32[(i * 4) + 3] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        tempaddr += 16;
    }

    WORD tag_word = 0;
    for (int i = 0; i < 8; i++) {
        int tagid = FPU_TAG_EMPTY;
        if (small_tag_word & (1 << i))
            tagid = fpu_get_tag_from_value(&m_fpu.st[i]);
        tag_word |= tagid << (i * 2);
    }
    m_fpu.tag_word = tag_word;
    return 0;
}

int CpuInternal::fpu_fwait(void) {
    if (m_fpu.status_word & 0x80) {
        if (m_cr[0] & 32) {
            cpu_exception(16, 0);
            return 1;
        } else {
            //pic_lower_irq(13);
            //pic_raise_irq(13); todo
        }
    }
    return 0;
}

void CpuInternal::fpu_init(void) {
}

double f80_to_double(floatx80* f80) {
    double scale_factor = pow(2.0, -63.0);
    WORD exponent;
    LARGE fraction;
    {
        exponent = (f80)->exp;
        fraction = (f80)->fraction;
    }
    double f = pow(2.0, ((exponent & 0x7FFF) - 0x3FFF));
    if (exponent > 0x8000)
        f = -f;
    f *= fraction * scale_factor;
    return f;
}

void CpuInternal::fpu_debug(void) {
}

void CpuInternal::printFloat80(floatx80* arg) {
    WORD exponent;
    LARGE fraction;
    {
        exponent = (arg)->exp;
        fraction = (arg)->fraction;
    }
    printf("%04x %08x%08x\n", exponent, (DWORD) (fraction >> 32), (DWORD) fraction);
}

void* CpuInternal::fpu_get_st_ptr1(void) {
    return &m_fpu.st[0];
}



//

void CpuInternal::float_raise(float_status_t* status, int flags) {
    status->float_exception_flags |= flags;
}

int CpuInternal::get_exception_flags(const float_status_t* status) {
    return status->float_exception_flags & ~status->float_suppress_exception;
}

int CpuInternal::float_exception_masked(const float_status_t* status, int flag) {
    return status->float_exception_masks & flag;
}

int CpuInternal::get_float_rounding_mode(const float_status_t* status) {
    return status->float_rounding_mode;
}

int CpuInternal::get_float_rounding_precision(const float_status_t* status) {
    return status->float_rounding_precision;
}

int CpuInternal::get_float_nan_handling_mode(const float_status_t* status) {
    return status->float_nan_handling_mode;
}

void CpuInternal::set_float_rounding_up(float_status_t* status) {
    status->float_exception_flags |= RAISE_SW_C1;
}

int CpuInternal::get_denormals_are_zeros(const float_status_t* status) {
    return status->denormals_are_zeros;
}

int CpuInternal::get_flush_underflow_to_zero(const float_status_t* status) {
    return status->flush_underflow_to_zero;
}

float32 CpuInternal::float32_round_to_int(float32 a, float_status_t* status) {
    return float32_round_to_int_with_scale(a, 0, status);
}

float32 CpuInternal::float32_fmadd(float32 a, float32 b, float32 c, float_status_t* status) {
    return float32_muladd(a, b, c, 0, status);
}

float32 CpuInternal::float32_fmsub(float32 a, float32 b, float32 c, float_status_t* status) {
    return float32_muladd(a, b, c, float_muladd_negate_c, status);
}

float32 CpuInternal::float32_fnmadd(float32 a, float32 b, float32 c, float_status_t* status) {
    return float32_muladd(a, b, c, float_muladd_negate_product, status);
}

float32 CpuInternal::float32_fnmsub(float32 a, float32 b, float32 c, float_status_t* status) {
    return float32_muladd(a, b, c, float_muladd_negate_result, status);
}

int CpuInternal::float32_compare(float32 a, float32 b, float_status_t* status) {
    return float32_compare_internal(a, b, 0, status);
}

int CpuInternal::float32_compare_quiet(float32 a, float32 b, float_status_t* status) {
    return float32_compare_internal(a, b, 1, status);
}

float64 CpuInternal::float64_round_to_int(float64 a, float_status_t* status) {
    return float64_round_to_int_with_scale(a, 0, status);
}

float64 CpuInternal::float64_fmadd(float64 a, float64 b, float64 c, float_status_t* status) {
    return float64_muladd(a, b, c, 0, status);
}

float64 CpuInternal::float64_fmsub(float64 a, float64 b, float64 c, float_status_t* status) {
    return float64_muladd(a, b, c, float_muladd_negate_c, status);
}

float64 CpuInternal::float64_fnmadd(float64 a, float64 b, float64 c, float_status_t* status) {
    return float64_muladd(a, b, c, float_muladd_negate_product, status);
}

float64 CpuInternal::float64_fnmsub(float64 a, float64 b, float64 c, float_status_t* status) {
    return float64_muladd(a, b, c, float_muladd_negate_result, status);
}

int CpuInternal::float64_compare(float64 a, float64 b, float_status_t* status) {
    return float64_compare_internal(a, b, 0, status);
}

int CpuInternal::float64_compare_quiet(float64 a, float64 b, float_status_t* status) {
    return float64_compare_internal(a, b, 1, status);
}

int CpuInternal::floatx80_compare(floatx80 a, floatx80 b, float_status_t* status) {
    return floatx80_compare_internal(a, b, 0, status);
}

int CpuInternal::floatx80_compare_quiet(floatx80 a, floatx80 b, float_status_t* status) {
    return floatx80_compare_internal(a, b, 1, status);
}

void CpuInternal::floatx80_abs(floatx80* reg) {
    reg->exp &= 0x7FFF;
}

void CpuInternal::floatx80_chs(floatx80* reg) {
    reg->exp ^= 0x8000;
}
