#include "cpu.h"
#include <cstdint>
#include <cstring>


int CpuInternal::float32_eq_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_equal);
}

int CpuInternal::float32_lt_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_less);
}

int CpuInternal::float32_le_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_equal);
}

int CpuInternal::float32_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_unordered);
}

int CpuInternal::float32_neq_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation != float_relation_equal);
}

int CpuInternal::float32_nlt_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation != float_relation_less);
}

int CpuInternal::float32_nle_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation != float_relation_less) && (relation != float_relation_equal);
}

int CpuInternal::float32_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation != float_relation_unordered);
}

int CpuInternal::float32_eq_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

int CpuInternal::float32_nge_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_unordered);
}

int CpuInternal::float32_ngt_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation != float_relation_greater);
}

int CpuInternal::float32_false_quiet(float32 a, float32 b, float_status_t* status) {
    float32_compare_quiet(a, b, status);
    return 0;
}

int CpuInternal::float32_neq_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

int CpuInternal::float32_ge_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_greater) || (relation == float_relation_equal);
}

int CpuInternal::float32_gt_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_greater);
}

int CpuInternal::float32_true_quiet(float32 a, float32 b, float_status_t* status) {
    float32_compare_quiet(a, b, status);
    return 1;
}

int CpuInternal::float32_eq_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_equal);
}

int CpuInternal::float32_lt_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_less);
}

int CpuInternal::float32_le_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_equal);
}

int CpuInternal::float32_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_unordered);
}

int CpuInternal::float32_neq_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation != float_relation_equal);
}

int CpuInternal::float32_nlt_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation != float_relation_less);
}

int CpuInternal::float32_nle_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation != float_relation_less) && (relation != float_relation_equal);
}

int CpuInternal::float32_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation != float_relation_unordered);
}

int CpuInternal::float32_eq_unordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

int CpuInternal::float32_nge_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_unordered);
}

int CpuInternal::float32_ngt_unordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation != float_relation_greater);
}

int CpuInternal::float32_false_signalling(float32 a, float32 b, float_status_t* status) {
    float32_compare(a, b, status);
    return 0;
}

int CpuInternal::float32_neq_ordered_signalling(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare(a, b, status);
    return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

int CpuInternal::float32_ge_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_greater) || (relation == float_relation_equal);
}

int CpuInternal::float32_gt_ordered_quiet(float32 a, float32 b, float_status_t* status) {
    int relation = float32_compare_quiet(a, b, status);
    return (relation == float_relation_greater);
}

int CpuInternal::float32_true_signalling(float32 a, float32 b, float_status_t* status) {
    float32_compare(a, b, status);
    return 1;
}

int CpuInternal::float64_eq_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_equal);
}

int CpuInternal::float64_lt_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_less);
}

int CpuInternal::float64_le_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_equal);
}

int CpuInternal::float64_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_unordered);
}

int CpuInternal::float64_neq_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation != float_relation_equal);
}

int CpuInternal::float64_nlt_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation != float_relation_less);
}

int CpuInternal::float64_nle_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation != float_relation_less) && (relation != float_relation_equal);
}

int CpuInternal::float64_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation != float_relation_unordered);
}

int CpuInternal::float64_eq_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

int CpuInternal::float64_nge_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_unordered);
}

int CpuInternal::float64_ngt_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation != float_relation_greater);
}

int CpuInternal::float64_false_quiet(float64 a, float64 b, float_status_t* status) {
    float64_compare_quiet(a, b, status);
    return 0;
}

int CpuInternal::float64_neq_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

int CpuInternal::float64_ge_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_greater) || (relation == float_relation_equal);
}

int CpuInternal::float64_gt_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_greater);
}

int CpuInternal::float64_true_quiet(float64 a, float64 b, float_status_t* status) {
    float64_compare_quiet(a, b, status);
    return 1;
}

int CpuInternal::float64_eq_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_equal);
}

int CpuInternal::float64_lt_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_less);
}

int CpuInternal::float64_le_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_equal);
}

int CpuInternal::float64_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_unordered);
}

int CpuInternal::float64_neq_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation != float_relation_equal);
}

int CpuInternal::float64_nlt_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation != float_relation_less);
}

int CpuInternal::float64_nle_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation != float_relation_less) && (relation != float_relation_equal);
}

int CpuInternal::float64_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation != float_relation_unordered);
}

int CpuInternal::float64_eq_unordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

int CpuInternal::float64_nge_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_less) || (relation == float_relation_unordered);
}

int CpuInternal::float64_ngt_unordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation != float_relation_greater);
}

int CpuInternal::float64_false_signalling(float64 a, float64 b, float_status_t* status) {
    float64_compare(a, b, status);
    return 0;
}

int CpuInternal::float64_neq_ordered_signalling(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare(a, b, status);
    return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

int CpuInternal::float64_ge_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_greater) || (relation == float_relation_equal);
}

int CpuInternal::float64_gt_ordered_quiet(float64 a, float64 b, float_status_t* status) {
    int relation = float64_compare_quiet(a, b, status);
    return (relation == float_relation_greater);
}

int CpuInternal::float64_true_signalling(float64 a, float64 b, float_status_t* status) {
    float64_compare(a, b, status);
    return 1;
}

int CpuInternal::cpu_sse_exception(void) {
    if ((m_cr[4] & (1 << 9)) == 0) {
        cpu_exception(6, 0);
        return 1;
    }

    if (m_cr[0] & 4) {
        cpu_exception(6, 0);
        return 1;
    }

    if (m_cr[0] & 8) {
        cpu_exception(7, 0);
        return 1;
    }

    return 0;
}

int CpuInternal::cpu_mmx_check(void) {
    if (m_cr[0] & 4) {
        cpu_exception(6, 0);
        return 1;
    }

    if (m_cr[0] & 8) {
        cpu_exception(7, 0);
        return 1;
    }

    if (fpu_fwait())
        return 1;

    m_fpu.ftop = 0;
    m_fpu.tag_word = 0;
    return 0;
}

void CpuInternal::cpu_update_mxcsr(void) {
    m_status.float_exception_flags = 0;
    m_status.float_nan_handling_mode = float_first_operand_nan;
    m_status.float_rounding_mode = m_mxcsr >> 13 & 3;
    m_status.flush_underflow_to_zero = (m_mxcsr >> 15) & (m_mxcsr >> 11) & 1;
    m_status.float_exception_masks = m_mxcsr >> 7 & 63;
    m_status.float_suppress_exception = 0;
    m_status.denormals_are_zeros = m_mxcsr >> 6 & 1;
}

int CpuInternal::cpu_sse_handle_exceptions(void) {
    int flags = m_status.float_exception_flags, unmasked = flags & ~m_status.float_exception_masks & 0x3F;
    m_status.float_exception_flags = 0;
    if (unmasked & 7)
        flags &= 7;
    m_mxcsr |= flags;
    if (unmasked) {
        if (m_cr[4] & (1 << 10)) {
            cpu_exception(19, 0);
            return 1;
        } else {
            cpu_exception(6, 0);
            return 1;
        }
    }
    return 0;
}

DWORD CpuInternal::cpu_get_linaddr2(DWORD i, struct decoded_instruction* j) {
    DWORD addr = m_reg32[i >> 8 & 15];
    addr += m_reg32[i >> 16 & 15] << (i >> 20 & 3);
    addr += j->disp32;
    return (addr & ((i << 12 & 65536) - 1)) + m_seg_base[i >> 22 & 7];
}

int CpuInternal::write_back_handler(void) {
    for (int i = 0; i < m_write_back_dwords; i++) {
        DWORD addr_ = m_write_back_linaddr + (i * 4), shift_ = m_tlb_shift_write, data_ = temp.d128[i],
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

int CpuInternal::get_read_ptr(DWORD flags, struct decoded_instruction* i, int dwords, int unaligned_exception) {
    DWORD linaddr = cpu_get_linaddr2(flags, i);
    if (linaddr & ((dwords << 2) - 1)) {
        if (unaligned_exception) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }

        for (int i = 0, j = 0; i < dwords; i++, j += 4) {
            DWORD addr_ = linaddr + j, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp.d128[i] = m_read_result;
                else
                    return 1;
            } else {
                temp.d128[i] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        m_result_ptr = temp.d128;
        m_write_back_dwords = dwords;
        m_write_back_linaddr = linaddr;
        return 0;
    }
    BYTE tag = m_tlb_tags[linaddr >> 12] >> m_tlb_shift_read;
    if (tag & 2) {
        if (cpu_mmu_translate(linaddr, m_tlb_shift_read))
            return 1;
    }

    auto phys = m_tlb[linaddr >> 12] + linaddr;
    if ((phys >= 0xA0000 && phys < 0xC0000) || (phys >= m_memory_size)) {
        for (int i = 0, j = 0; i < dwords; i++, j += 4)
            temp.d128[i] = m_mmio->Fetch(Astra::CPU::DataFormat::DWord, phys + j);
        m_result_ptr = temp.d128;
        m_write_back_dwords = dwords;
        m_write_back_linaddr = linaddr;
        return 0;
    }
    m_result_ptr = phys;
    return 0;
}

int CpuInternal::get_write_ptr(DWORD flags, struct decoded_instruction* i, int dwords, int unaligned_exception) {
    DWORD linaddr = cpu_get_linaddr2(flags, i);
    if (linaddr & ((dwords << 2) - 1)) {
        if (unaligned_exception) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }

        m_result_ptr = temp.d128;
        m_write_back = 1;
        m_write_back_dwords = dwords;
        m_write_back_linaddr = linaddr;
        return 0;
    }
    BYTE tag = m_tlb_tags[linaddr >> 12] >> m_tlb_shift_write;
    if (tag & 2) {
        if (cpu_mmu_translate(linaddr, m_tlb_shift_write))
            return 1;
    }
    auto phys = m_tlb[linaddr >> 12] + linaddr;
    if ((phys >= 0xA0000 && phys < 0xC0000) || (phys >= m_memory_size)) {
        m_write_back = 1;
        m_result_ptr = temp.d128;
        m_write_back_dwords = dwords;
        m_write_back_linaddr = linaddr;
        return 0;
    }
    m_write_back = 0;
    m_result_ptr = phys;
    return 0;
}

int CpuInternal::get_sse_read_ptr(DWORD flags, struct decoded_instruction* i, int dwords, int unaligned_exception) {
    if ((flags & (1 << 25))) {
        m_result_ptr = &m_xmm32[(flags >> 8 & 15) << 2];
        return 0;
    } else
        return get_read_ptr(flags, i, dwords, unaligned_exception);
}

int CpuInternal::get_sse_write_ptr(DWORD flags, struct decoded_instruction* i, int dwords, int unaligned_exception) {
    if ((flags & (1 << 25))) {
        m_result_ptr = &m_xmm32[(flags >> 8 & 15) << 2];
        m_write_back = 0;
        return 0;
    } else
        return get_write_ptr(flags, i, dwords, unaligned_exception);
}

int CpuInternal::get_mmx_read_ptr(DWORD flags, struct decoded_instruction* i, int dwords) {
    if ((flags & (1 << 25))) {
        m_result_ptr = &m_fpu.mm[flags >> 8 & 15].reg.r32[0];
        return 0;
    } else
        return get_read_ptr(flags, i, dwords, 0);
}

int CpuInternal::get_mmx_write_ptr(DWORD flags, struct decoded_instruction* i, int dwords) {
    if ((flags & (1 << 25))) {
        int reg = flags >> 8 & 15;
        m_result_ptr = &m_fpu.mm[reg].reg.r32[0];
        m_fpu.mm[reg].dummy = 0xFFFF;
        m_write_back = 0;
        return 0;
    } else
        return get_write_ptr(flags, i, dwords, 0);
}

int CpuInternal::get_reg_read_ptr(DWORD flags, struct decoded_instruction* i) {
    if ((flags & (1 << 25))) {
        m_result_ptr = &m_reg32[flags >> 8 & 15];
        return 0;
    } else
        return get_read_ptr(flags, i, 1, 0);
}

int CpuInternal::get_reg_write_ptr(DWORD flags, struct decoded_instruction* i) {
    if ((flags & (1 << 25))) {
        m_result_ptr = &m_reg32[flags >> 8 & 15];
        m_write_back = 0;
        return 0;
    } else
        return get_write_ptr(flags, i, 1, 0);
}

void* CpuInternal::get_mmx_reg_dest(int x) {
    m_fpu.mm[x].dummy = 0xFFFF;
    return &m_fpu.mm[x].reg;
}

void* CpuInternal::get_mmx_reg_src(int x) {
    return &m_fpu.mm[x].reg;
}

void* CpuInternal::get_sse_reg_dest(int x) {
    return &m_xmm32[(x) << 2];
}

void* CpuInternal::get_reg_dest(int x) {
    return &m_reg32[x];
}

void CpuInternal::punpckh(void* dst, int size, int copysize) {
    BYTE* dst8 = (BYTE*) dst;
    BYTE tmp[16];

    int idx = 0, nidx = 0;
    const int xormask = (size - 1) ^ (copysize - 1);
    while (idx < size) {
        for (int i = 0; i < copysize; i++)
            tmp[idx++ ^ xormask] = m_result_ptr.get<BYTE>((nidx + i) ^ xormask);
        for (int i = 0; i < copysize; i++)
            tmp[idx++ ^ xormask] = dst8[(nidx + i) ^ xormask];
        nidx += copysize;
    }
    memcpy(dst, tmp, size);
}

WORD CpuInternal::pack_i32_to_i16(DWORD x) {
    if (x >= 0x80000000) {
        if (x >= 0xFFFF8000)
            x &= 0xFFFF;
        else
            return 0x8000;
    } else {
        if (x > 0x7FFF)
            return 0x7FFF;
    }
    return x;
}

WORD CpuInternal::pack_i16_to_u8(int16_t x) {
    if (x >= 0xFF)
        return 0xFF;
    else if (x < 0)
        return 0;
    return x;
}

BYTE CpuInternal::pack_i16_to_i8(WORD x) {
    if (x >= 0x8000) {
        if (x >= 0xFF80)
            x &= 0xFF;
        else
            return 0x80;
    } else {
        if (x > 0x7F)
            return 0x7F;
    }
    return x;
}

void CpuInternal::packssdw(void* dest, int dwordcount) {
    WORD res[8];
    DWORD* dest32 = (DWORD*) dest;
    for (int i = 0; i < dwordcount; i++) {
        res[i] = pack_i32_to_i16(dest32[i]);
        res[i | dwordcount] = pack_i32_to_i16(m_result_ptr.get<DWORD>(i * sizeof(DWORD)));
    }
    memcpy(dest, res, dwordcount << 2);
}

void CpuInternal::punpckl(void* dst, int size, int copysize) {
    BYTE* dst8 = (BYTE*) dst;
    BYTE tmp[16];
    int idx = 0, nidx = 0;

    while (idx < size) {
        for (int i = 0; i < copysize; i++)
            tmp[idx++] = dst8[(nidx + i)];
        for (int i = 0; i < copysize; i++)
            tmp[idx++] = m_result_ptr.get<BYTE>(nidx + i);
        nidx += copysize;
    }

    memcpy(dst, tmp, size);
}

void CpuInternal::psubsb(BYTE* dest, int bytecount) {
    for (int i = 0; i < bytecount; i++) {
        BYTE x = dest[i];
        BYTE y = m_result_ptr.get<BYTE>(i);
        BYTE res = x - y;

        x = (x >> 7) + 0x7F;
        if ((int8_t) ((x ^ y) & (x ^ res)) < 0)
            res = x;
        dest[i] = res;
    }
}

void CpuInternal::psubsw(WORD* dest, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        WORD x = dest[i];
        WORD y = m_result_ptr.get<WORD>(i * sizeof(WORD));
        WORD res = x - y;

        x = (x >> 15) + 0x7FFF;
        if ((int16_t) ((x ^ y) & (x ^ res)) < 0)
            res = x;
        dest[i] = res;
    }
}

void CpuInternal::pminub(BYTE* dest, int bytecount) {
    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        if (src < dest[i])
            dest[i] = src;
    }
}

void CpuInternal::pmaxub(BYTE* dest, int bytecount) {
    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        if (dest[i] < src)
            dest[i] = src;
    }
}

void CpuInternal::pminsw(int16_t* dest, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<int16_t>(i * sizeof(int16_t));
        if (src < dest[i])
            dest[i] = src;
    }
}

void CpuInternal::pmaxsw(int16_t* dest, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<int16_t>(i * sizeof(int16_t));
        if (src > dest[i])
            dest[i] = src;
    }
}

void CpuInternal::paddsb(BYTE* dest, int bytecount) {
    for (int i = 0; i < bytecount; i++) {
        BYTE x = dest[i];
        BYTE y = m_result_ptr.get<BYTE>(i);
        BYTE res = x + y;

        x = (x >> 7) + 0x7F;
        if ((int8_t) ((x ^ y) | ~(y ^ res)) >= 0)
            res = x;
        dest[i] = res;
    }
}

void CpuInternal::paddsw(WORD* dest, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        WORD x = dest[i];
        WORD y = m_result_ptr.get<WORD>(i * sizeof(WORD));
        WORD res = x + y;

        x = (x >> 15) + 0x7FFF;
        if ((int16_t) ((x ^ y) | ~(y ^ res)) >= 0)
            res = x;
        dest[i] = res;
    }
}

void CpuInternal::pshuf(void* dest, const ResultPtr& src, int imm, int shift) {
    BYTE res[16];
    int id = 0;
    for (int i = 0; i < 4; i++) {
        int index = imm & 3, index4 = index << shift;
        if (shift == 2) {
            res[id + 0] = src.get<BYTE>(index4 + 0);
            res[id + 1] = src.get<BYTE>(index4 + 1);
            res[id + 2] = src.get<BYTE>(index4 + 2);
            res[id + 3] = src.get<BYTE>(index4 + 3);
            id += 4;
        } else {
            res[id + 0] = src.get<BYTE>(index4 + 0);
            res[id + 1] = src.get<BYTE>(index4 + 1);
            id += 2;
        }
        imm >>= 2;
    }
    memcpy(dest, res, 4 << shift);
}

void CpuInternal::pshufb(void* dest, int bytes) {
    BYTE res[16], * dest8 = (BYTE*) dest;
    int mask = bytes - 1;

    for (int i = 0; i < bytes; i++) {
        auto src = m_result_ptr.get<int8_t>(i * sizeof(int8_t));
        res[i] = src < 0 ? 0 : dest8[src & mask];
    }

    memcpy(dest, res, bytes);
}

void CpuInternal::cpu_psraw(WORD* a, int shift, int mask, int wordcount) {
    for (int i = 0; i < wordcount; i++)
        a[i] = (int16_t) a[i] >> shift & mask;
}

void CpuInternal::cpu_psrlw(WORD* a, int shift, int mask, int wordcount) {
    for (int i = 0; i < wordcount; i++)
        a[i] = a[i] >> shift & mask;
}

void CpuInternal::cpu_psllw(WORD* a, int shift, int mask, int wordcount) {
    for (int i = 0; i < wordcount; i++)
        a[i] = a[i] << shift & mask;
}

void CpuInternal::cpu_psrad(DWORD* a, int shift, int mask, int wordcount) {
    int dwordcount = wordcount >> 1;
    for (int i = 0; i < dwordcount; i++)
        a[i] = (int32_t) a[i] >> shift & mask;
}

void CpuInternal::cpu_psrld(DWORD* a, int shift, int mask, int wordcount) {
    int dwordcount = wordcount >> 1;
    for (int i = 0; i < dwordcount; i++)
        a[i] = a[i] >> shift & mask;
}

void CpuInternal::cpu_pslld(DWORD* a, int shift, int mask, int wordcount) {
    int dwordcount = wordcount >> 1;
    for (int i = 0; i < dwordcount; i++)
        a[i] = a[i] << shift & mask;
}

void CpuInternal::cpu_psrlq(LARGE* a, int shift, int mask, int wordcount) {
    int qwordcount = wordcount >> 2;
    for (int i = 0; i < qwordcount; i++) {
        if (mask)
            a[i] = a[i] >> shift;
        else
            a[i] = 0;
    }
}

void CpuInternal::cpu_psllq(LARGE* a, int shift, int mask, int wordcount) {
    int qwordcount = wordcount >> 2;
    for (int i = 0; i < qwordcount; i++)
        if (mask)
            a[i] = a[i] << shift;
        else
            a[i] = 0;
}

void CpuInternal::cpu_pslldq(LARGE* a, int shift, int mask) {
    if (mask == 0) {
        a[0] = 0;
        a[1] = 0;
        return;
    }
    if (shift == 64) {
        a[1] = a[0];
        a[0] = 0;
    } else if (shift > 64) {
        a[1] = a[0] << (shift - 64L);
        a[0] = 0;
    } else {
        a[0] <<= shift;
        a[0] |= a[1] >> (64L - shift);
        a[1] <<= shift;
    }
}

void CpuInternal::cpu_psrldq(LARGE* a, int shift, int mask) {
    if (mask == 0) {
        a[0] = 0;
        a[1] = 0;
        return;
    }
    if (shift == 64) {
        a[0] = a[1];
        a[1] = 0;
    } else if (shift > 64) {
        a[0] = a[1] >> (shift - 64L);
        a[1] = 0;
    } else {
        a[0] >>= shift;
        a[0] |= a[1] << (64L - shift);
        a[1] >>= shift;
    }
}

void CpuInternal::pcmpeqb(BYTE* dest, int count) {
    for (int i = 0; i < count; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        if (src == dest[i])
            dest[i] = 0xFF;
        else
            dest[i] = 0;
    }
}

void CpuInternal::pcmpeqw(WORD* dest, int count) {
    for (int i = 0; i < count; i++) {
        auto src = m_result_ptr.get<WORD>(i * sizeof(WORD));
        if (src == dest[i])
            dest[i] = 0xFFFF;
        else
            dest[i] = 0;
    }
}

void CpuInternal::pcmpeqd(DWORD* dest, int count) {
    for (int i = 0; i < count; i++) {
        auto src = m_result_ptr.get<DWORD>(i * sizeof(DWORD));
        if (src == dest[i])
            dest[i] = 0xFFFFFFFF;
        else
            dest[i] = 0;
    }
}

void CpuInternal::pcmpgtb(int8_t* dest, int count) {
    for (int i = 0; i < count; i++) {
        auto src = m_result_ptr.get<int8_t>(i * sizeof(int8_t));
        if (dest[i] > src)
            dest[i] = 0xFF;
        else
            dest[i] = 0;
    }
}

void CpuInternal::pcmpgtw(int16_t* dest, int count) {
    for (int i = 0; i < count; i++) {
        auto src = m_result_ptr.get<int16_t>(i * sizeof(int16_t));
        if (dest[i] > src)
            dest[i] = 0xFFFF;
        else
            dest[i] = 0;
    }
}

void CpuInternal::pcmpgtd(int32_t* dest, int count) {
    for (int i = 0; i < count; i++) {
        auto src = m_result_ptr.get<int32_t>(i * sizeof(int32_t));
        if (dest[i] > src)
            dest[i] = 0xFFFFFFFF;
        else
            dest[i] = 0;
    }
}

void CpuInternal::packuswb(void* dest, int wordcount) {
    BYTE res[16];
    WORD* dest16 = (WORD*) dest;

    for (int i = 0; i < wordcount; i++) {
        res[i] = pack_i16_to_u8(dest16[i]);
        res[i | wordcount] = pack_i16_to_u8(m_result_ptr.get<WORD>(i * sizeof(WORD)));
    }

    memcpy(dest, res, wordcount << 1);
}

void CpuInternal::packsswb(void* dest, int wordcount) {
    BYTE res[16];
    WORD* dest16 = (WORD*) dest;

    for (int i = 0; i < wordcount; i++) {
        res[i] = pack_i16_to_i8(dest16[i]);
        res[i | wordcount] = pack_i16_to_i8(m_result_ptr.get<WORD>(i * sizeof(WORD)));
    }

    memcpy(dest, res, wordcount << 1);
}

void CpuInternal::pmullw(WORD* dest, int wordcount, int shift) {
    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<int16_t>(i * sizeof(int16_t));
        DWORD result = (DWORD) (int16_t) dest[i] * (DWORD) src;
        dest[i] = result >> shift;
    }
}

void CpuInternal::pmuluw(void* dest, int wordcount, int shift) {
    WORD* dest16 = (WORD*) dest;

    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<WORD>(i * sizeof(WORD));
        DWORD result = (DWORD) dest16[i] * (DWORD) src;
        dest16[i] = result >> shift;
    }
}

void CpuInternal::pmuludq(void* dest, int dwordcount) {
    DWORD* dest32 = (DWORD*) dest;

    for (int i = 0; i < dwordcount; i += 2) {
        auto src = m_result_ptr.get<DWORD>(i * sizeof(DWORD));
        LARGE result = (LARGE) dest32[i] * (LARGE) src;
        dest32[i] = result;
        dest32[i + 1] = result >> 32L;
    }
}

int CpuInternal::pmovmskb(int bytecount) {
    int dest = 0;
    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        dest |= (src >> 7) << i;
    }
    return dest;
}

void CpuInternal::psubusb(BYTE* dest, int bytecount) {
    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        BYTE result = dest[i] - src;
        dest[i] = -(result <= dest[i]) & result;
    }
}

void CpuInternal::psubusw(WORD* dest, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<WORD>(i * sizeof(WORD));
        WORD result = dest[i] - src;
        dest[i] = -(result <= dest[i]) & result;
    }
}

void CpuInternal::paddusb(BYTE* dest, int bytecount) {
    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        BYTE result = dest[i] + src;
        dest[i] = -(result < dest[i]) | result;
    }
}

void CpuInternal::paddusw(WORD* dest, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<WORD>(i * sizeof(WORD));
        WORD result = dest[i] + src;
        dest[i] = -(result < dest[i]) | result;
    }
}

void CpuInternal::paddb(BYTE* dest, int bytecount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < bytecount; i++)
            dest[i] <<= 1;
    else
        for (int i = 0; i < bytecount; i++)
            dest[i] += m_result_ptr.get<BYTE>(i);
}

void CpuInternal::paddw(WORD* dest, int wordcount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < wordcount; i++)
            dest[i] <<= 1;
    else
        for (int i = 0; i < wordcount; i++)
            dest[i] += m_result_ptr.get<WORD>(i * sizeof(WORD));
}

void CpuInternal::paddd(DWORD* dest, int dwordcount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < dwordcount; i++)
            dest[i] <<= 1;
    else
        for (int i = 0; i < dwordcount; i++)
            dest[i] += m_result_ptr.get<DWORD>(i * sizeof(DWORD));
}

void CpuInternal::psubb(BYTE* dest, int bytecount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < bytecount; i++)
            dest[i] >>= 1;
    else
        for (int i = 0; i < bytecount; i++)
            dest[i] -= m_result_ptr.get<BYTE>(i);
}

void CpuInternal::psubw(WORD* dest, int wordcount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < wordcount; i++)
            dest[i] >>= 1;
    else
        for (int i = 0; i < wordcount; i++)
            dest[i] -= m_result_ptr.get<WORD>(i * sizeof(WORD));
}

void CpuInternal::psubd(DWORD* dest, int dwordcount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < dwordcount; i++)
            dest[i] >>= 1;
    else
        for (int i = 0; i < dwordcount; i++)
            dest[i] -= m_result_ptr.get<DWORD>(i * sizeof(DWORD));
}

void CpuInternal::psubq(LARGE* dest, int qwordcount) {
    if (m_result_ptr == dest)
        for (int i = 0; i < qwordcount; i++)
            dest[i] >>= 1;
    else
        for (int i = 0; i < qwordcount; i++)
            dest[i] -= m_result_ptr.get<LARGE>(i * sizeof(LARGE));
}

DWORD CpuInternal::cmpps(float32 dest, float32 src, int cmp) {
    switch (cmp & 7) {
        case 0:
            return -float32_eq_ordered_quiet(dest, src, &m_status);
        case 1:
            return -float32_lt_ordered_signalling(dest, src, &m_status);
        case 2:
            return -float32_le_ordered_signalling(dest, src, &m_status);
        case 3:
            return -float32_unordered_quiet(dest, src, &m_status);
        case 4:
            return -float32_neq_ordered_quiet(dest, src, &m_status);
        case 5:
            return -float32_nlt_unordered_signalling(dest, src, &m_status);
        case 6:
            return -float32_nle_unordered_signalling(dest, src, &m_status);
        case 7:
            return -float32_ordered_quiet(dest, src, &m_status);
    }

    util_abort();
    return 0;
}

LARGE CpuInternal::cmppd(float64 dest, float64 src, int cmp) {
    switch (cmp & 7) {
        case 0:
            return -float64_eq_ordered_quiet(dest, src, &m_status);
        case 1:
            return -float64_lt_ordered_signalling(dest, src, &m_status);
        case 2:
            return -float64_le_ordered_signalling(dest, src, &m_status);
        case 3:
            return -float64_unordered_quiet(dest, src, &m_status);
        case 4:
            return -float64_neq_ordered_quiet(dest, src, &m_status);
        case 5:
            return -float64_nlt_unordered_signalling(dest, src, &m_status);
        case 6:
            return -float64_nle_unordered_signalling(dest, src, &m_status);
        case 7:
            return -float64_ordered_quiet(dest, src, &m_status);
    }

    util_abort();
    return 0;
}

void CpuInternal::shufps(void* dest, int imm) {
    DWORD * dest32 = (DWORD*) dest;
    DWORD res[4];

    res[0] = dest32[imm >> 0 & 3];
    res[1] = dest32[imm >> 2 & 3];
    res[2] = m_result_ptr.get<DWORD>((imm >> 4 & 3) * sizeof(DWORD));
    res[3] = m_result_ptr.get<DWORD>((imm >> 6 & 3) * sizeof(DWORD));

    memcpy(dest32, res, 16);
}

void CpuInternal::shufpd(void* dest, int imm) {
    DWORD * dest32 = (DWORD*) dest;

    if (imm & 1) {
        dest32[0] = dest32[2];
        dest32[1] = dest32[3];
    }
    if (imm & 2) {
        dest32[2] = m_result_ptr.get<DWORD>(2 * sizeof(DWORD));
        dest32[3] = m_result_ptr.get<DWORD>(3 * sizeof(DWORD));
    } else {
        dest32[2] = dest32[0];
        dest32[3] = dest32[1];
    }
}

void CpuInternal::pavgb(void* dest, int bytecount) {
    BYTE* dest8 = (BYTE*) dest;

    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<BYTE>(i);
        dest8[i] = (dest8[i] + src) >> 1;
    }
}

void CpuInternal::pavgw(void* dest, int wordcount) {
    WORD* dest16 = (WORD*) dest;

    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<WORD>(i * sizeof(WORD));
        dest16[i] = (dest16[i] + src) >> 1;
    }
}

void CpuInternal::pmaddwd(void* dest, int dwordcount) {
    WORD * dest16 = (WORD*) dest;
    DWORD res[4];

    int idx = 0;
    for (int i = 0; i < dwordcount; i++) {
        auto src = m_result_ptr.get<WORD>(idx * sizeof(WORD));
        auto src1 = m_result_ptr.get<WORD>((idx + 1) * sizeof(WORD));

        res[i] = ((DWORD) (int16_t) src * (DWORD) (int16_t) dest16[idx]) +
                 ((DWORD) (int16_t) src1 * (DWORD) (int16_t) dest16[idx + 1]);
        idx += 2;
    }
    memcpy(dest, res, dwordcount << 2);
}

void CpuInternal::psadbw(void* dest, int qwordcount) {
    BYTE* dest8 = (BYTE*) dest;

    for (int i = 0; i < qwordcount; i++) {
        DWORD sum = 0, offs = i << 3;

        for (int j = 0; j < 8; j++) {
            auto src = m_result_ptr.get<BYTE>(j | offs);

            int diff = src - dest8[j | offs];
            if (diff < 0)
                diff = -diff;
            sum += diff;
            dest8[j | offs] = 0;
        }

        dest8[offs | 0] = sum;
        dest8[offs | 1] = sum >> 8;
    }
}

void CpuInternal::pabsb(void* dest, int bytecount) {
    BYTE* dest8 = (BYTE*) dest;

    for (int i = 0; i < bytecount; i++) {
        auto src = m_result_ptr.get<int8_t>(i * sizeof(int8_t));
        dest8[i] = src < 0 ? -src : src;
    }
}

void CpuInternal::pabsw(void* dest, int wordcount) {
    WORD* dest16 = (WORD*) dest;

    for (int i = 0; i < wordcount; i++) {
        auto src = m_result_ptr.get<int16_t>(i * sizeof(int16_t));
        dest16[i] = src < 0 ? -src : src;
    }
}

void CpuInternal::pabsd(void* dest, int dwordcount) {
    DWORD* dest32 = (DWORD*) dest;

    for (int i = 0; i < dwordcount; i++) {
        auto src = m_result_ptr.get<int32_t>(i * sizeof(int32_t));
        dest32[i] = src < 0 ? -src : src;
    }
}

int CpuInternal::execute_0F10_17(struct decoded_instruction* i) {
    if (cpu_sse_exception())
        return 1;
    DWORD* dest32, * src32, flags = i->flags;
    switch (i->imm8 & 31) {
        case MOVUPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            dest32[2] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case MOVSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            if (!(flags & (1 << 25))) {
                dest32[1] = 0;
                dest32[2] = 0;
                dest32[3] = 0;
            }
            break;
        case MOVSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            if (!(flags & (1 << 25))) {
                dest32[2] = 0;
                dest32[3] = 0;
            }
            break;
        case MOVUPS_XEoXGo:
            if ((get_sse_write_ptr(flags, i, 4, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            m_result_ptr.set<DWORD>(dest32[2], 8);
            m_result_ptr.set<DWORD>(dest32[3], 12);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVSS_XEdXGd:
            if ((get_sse_write_ptr(flags, i, 1, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVSD_XEqXGq:
            if ((get_sse_write_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVHLPS_XGqXEq:
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            src32 = (DWORD*) get_sse_reg_dest(flags >> 8 & 15);
            dest32[0] = src32[2];
            dest32[1] = src32[3];
            break;
        case MOVLPS_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            break;
        case UNPCKLPS_XGoXEq:
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[2] = dest32[1];
            dest32[1] = m_result_ptr.get<DWORD>();
            dest32[3] = m_result_ptr.get<DWORD>(4);
            break;
        case UNPCKLPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[2] = m_result_ptr.get<DWORD>();
            dest32[3] = m_result_ptr.get<DWORD>(4);
            break;
        case UNPCKHPS_XGoXEq:
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = dest32[2];
            dest32[2] = dest32[3];
            dest32[1] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case UNPCKHPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = dest32[2];
            dest32[1] = dest32[3];
            dest32[2] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case MOVLHPS_XGqXEq:
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            src32 = (DWORD*) get_sse_reg_dest(flags >> 8 & 15);
            dest32[2] = src32[0];
            dest32[3] = src32[1];
            break;
        case MOVHPS_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[2] = m_result_ptr.get<DWORD>();
            dest32[3] = m_result_ptr.get<DWORD>(4);
            break;
        case MOVSHDUP_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>(4);
            dest32[1] = m_result_ptr.get<DWORD>(4);
            dest32[2] = m_result_ptr.get<DWORD>(12);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case MOVHPS_XEqXGq:
            if ((get_sse_write_ptr(flags, i, 2, 1)))
                return 1;
            src32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            if ((flags & (1 << 25))) {
                m_result_ptr.set<DWORD>(src32[0], 8);
                m_result_ptr.set<DWORD>(src32[1], 12);
            } else {
                m_result_ptr.set<DWORD>(src32[2]);
                m_result_ptr.set<DWORD>(src32[3], 4);
            }
            if (m_write_back && write_back_handler())
                return 1;
            break;
    }
    return 0;
}

int CpuInternal::execute_0F28_2F(struct decoded_instruction* i) {
    if (cpu_sse_exception())
        return 1;
    DWORD* dest32, * src32, flags = i->flags;
    int fp_exception = 0;
    switch (i->imm8 & 15) {
        case MOVAPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            dest32[2] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case MOVAPS_XEoXGo:
            if ((get_sse_write_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            m_result_ptr.set<DWORD>(dest32[2], 8);
            m_result_ptr.set<DWORD>(dest32[3], 12);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case CVTPI2PS_XGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = int32_to_float32(m_result_ptr.get<DWORD>(), &m_status);
            dest32[1] = int32_to_float32(m_result_ptr.get<DWORD>(4), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTSI2SS_XGdEd:
            if ((get_reg_read_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = int32_to_float32(m_result_ptr.get<DWORD>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTPI2PD_XGoMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            *(LARGE*) (&dest32[0]) = int32_to_float64(m_result_ptr.get<DWORD>());
            *(LARGE*) (&dest32[2]) = int32_to_float64(m_result_ptr.get<DWORD>(4));
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTSI2SD_XGqMEd:
            if ((get_reg_read_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            *(LARGE*) (&dest32[0]) = int32_to_float64(m_result_ptr.get<DWORD>());
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTPS2PI_MGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            if (i->imm8 & 16) {
                dest32[0] = float32_to_int32(m_result_ptr.get<DWORD>(), &m_status);
                dest32[1] = float32_to_int32(m_result_ptr.get<DWORD>(4), &m_status);
            } else {
                dest32[0] = float32_to_int32_round_to_zero(m_result_ptr.get<DWORD>(), &m_status);
                dest32[1] = float32_to_int32_round_to_zero(m_result_ptr.get<DWORD>(4), &m_status);
            }
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTSS2SI_GdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_reg_dest(flags >> 12 & 15);
            if (i->imm8 & 16)
                dest32[0] = float32_to_int32(m_result_ptr.get<DWORD>(), &m_status);
            else
                dest32[0] = float32_to_int32_round_to_zero(m_result_ptr.get<DWORD>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTPD2PI_MGqXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            if (i->imm8 & 16) {
                dest32[0] = float64_to_int32(m_result_ptr.get<LARGE>(), &m_status);
                dest32[1] = float64_to_int32(m_result_ptr.get<LARGE>(8), &m_status);
            } else {
                dest32[0] = float64_to_int32_round_to_zero(m_result_ptr.get<LARGE>(), &m_status);
                dest32[1] = float64_to_int32_round_to_zero(m_result_ptr.get<LARGE>(8), &m_status);
            }
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTSD2SI_GdXEq:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_reg_dest(flags >> 12 & 15);
            if (i->imm8 & 16)
                dest32[0] = float64_to_int32(m_result_ptr.get<LARGE>(), &m_status);
            else
                dest32[0] = float64_to_int32_round_to_zero(m_result_ptr.get<LARGE>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case UCOMISS_XGdXEd: {
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            int result;
            if (i->imm8 & 16)
                result = float32_compare(dest32[0], m_result_ptr.get<DWORD>(), &m_status);
            else
                result = float32_compare_quiet(dest32[0], m_result_ptr.get<DWORD>(), &m_status);
            int eflags = 0;
            switch (result) {
                case float_relation_unordered:
                    eflags = 0x40 | 0x04 | 0x01;
                    break;
                case float_relation_less:
                    eflags = 0x01;
                    break;
                case float_relation_greater:
                    eflags = 0;
                    break;
                case float_relation_equal:
                    eflags = 0x40;
                    break;
            }
            cpu_set_eflags(eflags | (m_eflags & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01)));
            fp_exception = cpu_sse_handle_exceptions();
            break;
        }
        case UCOMISD_XGqXEq: {
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            int result;
            if (i->imm8 & 16)
                result = float64_compare(*(LARGE*) &dest32[0], m_result_ptr.get<LARGE>(), &m_status);
            else
                result = float64_compare_quiet(*(LARGE*) &dest32[0], m_result_ptr.get<LARGE>(), &m_status);
            int eflags = 0;
            switch (result) {
                case float_relation_unordered:
                    eflags = 0x40 | 0x04 | 0x01;
                    break;
                case float_relation_less:
                    eflags = 0x01;
                    break;
                case float_relation_greater:
                    eflags = 0;
                    break;
                case float_relation_equal:
                    eflags = 0x40;
                    break;
            }
            cpu_set_eflags(eflags | (m_eflags & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01)));
            fp_exception = cpu_sse_handle_exceptions();
            break;
        }
    }
    return fp_exception;
}

float32 CpuInternal::rsqrt(float32 a) {
    return float32_div(m_float32_one, float32_sqrt(a, &m_status), &m_status);
}

float32 CpuInternal::rcp(float32 a) {
    return float32_div(m_float32_one, a, &m_status);
}

int CpuInternal::execute_0F50_57(struct decoded_instruction* i) {
    if (cpu_sse_exception())
        return 1;
    DWORD* dest32, flags = i->flags;
    int fp_exception = 0, result;
    switch (i->imm8 & 15) {
        case MOVMSKPS_GdXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            result = 0;
            result = m_result_ptr.get<DWORD>() >> 31;
            result |= m_result_ptr.get<DWORD>(4) >> 30 & 2;
            result |= m_result_ptr.get<DWORD>(8) >> 29 & 4;
            result |= m_result_ptr.get<DWORD>(12) >> 28 & 8;
            m_reg32[flags >> 12 & 15] = result;
            break;
        case MOVMSKPD_GdXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            result = 0;
            result = m_result_ptr.get<DWORD>(4) >> 31;
            result |= m_result_ptr.get<DWORD>(12) >> 30 & 2;
            m_reg32[flags >> 12 & 15] = result;
            break;
        case SQRTPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_sqrt(m_result_ptr.get<DWORD>(), &m_status);
            dest32[1] = float32_sqrt(m_result_ptr.get<DWORD>(4), &m_status);
            dest32[2] = float32_sqrt(m_result_ptr.get<DWORD>(8), &m_status);
            dest32[3] = float32_sqrt(m_result_ptr.get<DWORD>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SQRTSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_sqrt(m_result_ptr.get<DWORD>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SQRTPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            *(LARGE*) &dest32[0] = float64_sqrt(m_result_ptr.get<LARGE>(), &m_status);
            *(LARGE*) &dest32[2] = float64_sqrt(m_result_ptr.get<LARGE>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SQRTSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            *(LARGE*) &dest32[0] = float64_sqrt(m_result_ptr.get<LARGE>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case RSQRTSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = rsqrt(m_result_ptr.get<DWORD>());
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case RSQRTPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = rsqrt(m_result_ptr.get<DWORD>());
            dest32[1] = rsqrt(m_result_ptr.get<DWORD>(4));
            dest32[2] = rsqrt(m_result_ptr.get<DWORD>(8));
            dest32[3] = rsqrt(m_result_ptr.get<DWORD>(12));
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case RCPSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = rcp(m_result_ptr.get<DWORD>());
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case RCPPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = rcp(m_result_ptr.get<DWORD>());
            dest32[1] = rcp(m_result_ptr.get<DWORD>(4));
            dest32[2] = rcp(m_result_ptr.get<DWORD>(8));
            dest32[3] = rcp(m_result_ptr.get<DWORD>(12));
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case ANDPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] &= m_result_ptr.get<DWORD>();
            dest32[1] &= m_result_ptr.get<DWORD>(4);
            dest32[2] &= m_result_ptr.get<DWORD>(8);
            dest32[3] &= m_result_ptr.get<DWORD>(12);
            break;
        case ORPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] |= m_result_ptr.get<DWORD>();
            dest32[1] |= m_result_ptr.get<DWORD>(4);
            dest32[2] |= m_result_ptr.get<DWORD>(8);
            dest32[3] |= m_result_ptr.get<DWORD>(12);
            break;
        case ANDNPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = ~dest32[0] & m_result_ptr.get<DWORD>();
            dest32[1] = ~dest32[1] & m_result_ptr.get<DWORD>(4);
            dest32[2] = ~dest32[2] & m_result_ptr.get<DWORD>(8);
            dest32[3] = ~dest32[3] & m_result_ptr.get<DWORD>(12);
            break;
        case XORPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] ^= m_result_ptr.get<DWORD>();
            dest32[1] ^= m_result_ptr.get<DWORD>(4);
            dest32[2] ^= m_result_ptr.get<DWORD>(8);
            dest32[3] ^= m_result_ptr.get<DWORD>(12);
            break;
    }
    return fp_exception;
}

int CpuInternal::execute_0F68_6F(struct decoded_instruction* i) {
    DWORD* dest32, flags = i->flags;
    switch (i->imm8 & 15) {
        case PUNPCKHBW_MGqMEq:
            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;

            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 8, 1);
            break;
        case PUNPCKHBW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 16, 1);
            break;
        case PUNPCKHWD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 8, 2);
            break;
        case PUNPCKHWD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 16, 2);
            break;
        case PUNPCKHDQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 8, 4);
            break;
        case PUNPCKHDQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 16, 4);
            break;
        case PACKSSDW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            packssdw(dest32, 2);
            break;
        case PACKSSDW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            packssdw(dest32, 4);
            break;
        case PUNPCKLQDQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 16, 8);
            break;
        case PUNPCKHQDQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckh(dest32, 16, 8);
            break;
        case MOVD_MGdEd:

            if (cpu_mmx_check())
                return 1;
            if ((get_reg_read_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = 0;
            break;
        case MOVD_XGdEd:
            if (cpu_sse_exception())
                return 1;
            if ((get_reg_read_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = 0;
            dest32[2] = 0;
            dest32[3] = 0;
            break;
        case MOVQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            break;
        case MOVDQA_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            dest32[2] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case MOVDQU_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            dest32[2] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            break;
        case OP_68_6F_INVALID: {
            cpu_exception(6, 0);
            return 1;
        }
    }
    return 0;
}

int CpuInternal::execute_0FE8_EF(struct decoded_instruction* i) {
    DWORD* dest32, flags = i->flags;
    if (i->imm8 & 1) {
        if (cpu_sse_exception())
            return 1;
    } else {
        if (cpu_mmx_check())
            return 1;
    }
    switch (i->imm8 & 15) {
        case PSUBSB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubsb((BYTE*) dest32, 8);
            break;
        case PSUBSB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubsb((BYTE*) dest32, 16);
            break;
        case PSUBSW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubsw((WORD*) dest32, 4);
            break;
        case PSUBSW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubsw((WORD*) dest32, 8);
            break;
        case PMINSW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pminsw((int16_t*) dest32, 4);
            break;
        case PMINSW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pminsw((int16_t*) dest32, 8);
            break;
        case POR_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] |= m_result_ptr.get<DWORD>();
            dest32[1] |= m_result_ptr.get<DWORD>(4);
            break;
        case POR_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] |= m_result_ptr.get<DWORD>();
            dest32[1] |= m_result_ptr.get<DWORD>(4);
            dest32[2] |= m_result_ptr.get<DWORD>(8);
            dest32[3] |= m_result_ptr.get<DWORD>(12);
            break;
        case PADDSB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddsb((BYTE*) dest32, 8);
            break;
        case PADDSB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddsb((BYTE*) dest32, 16);
            break;
        case PADDSW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddsw((WORD*) dest32, 4);
            break;
        case PADDSW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddsw((WORD*) dest32, 8);
            break;
        case PMAXSW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmaxsw((int16_t*) dest32, 4);
            break;
        case PMAXSW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmaxsw((int16_t*) dest32, 8);
            break;
        case PXOR_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] ^= m_result_ptr.get<DWORD>();
            dest32[1] ^= m_result_ptr.get<DWORD>(4);
            break;
        case PXOR_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] ^= m_result_ptr.get<DWORD>();
            dest32[1] ^= m_result_ptr.get<DWORD>(4);
            dest32[2] ^= m_result_ptr.get<DWORD>(8);
            dest32[3] ^= m_result_ptr.get<DWORD>(12);
            break;
    }
    return 0;
}

void CpuInternal::pshift(void* dest, int opcode, int wordcount, int imm) {
    int mask = -1;
    switch (opcode) {
        case PSHIFT_PSRLW:
            if (imm >= 16)
                mask = 0;
            cpu_psrlw((WORD*) dest, imm & 15, mask, wordcount);
            break;
        case PSHIFT_PSRAW:
            if (imm >= 16)
                mask = 0;
            cpu_psraw((WORD*) dest, imm & 15, mask, wordcount);
            break;
        case PSHIFT_PSLLW:
            if (imm >= 16)
                mask = 0;
            cpu_psllw((WORD*) dest, imm & 15, mask, wordcount);
            break;
        case PSHIFT_PSRLD:
            if (imm >= 32)
                mask = 0;
            cpu_psrld((DWORD*) dest, imm & 31, mask, wordcount);
            break;
        case PSHIFT_PSRAD:
            if (imm >= 32)
                mask = 0;
            cpu_psrad((DWORD*) dest, imm & 31, mask, wordcount);
            break;
        case PSHIFT_PSLLD:
            if (imm >= 32)
                mask = 0;
            cpu_pslld((DWORD*) dest, imm & 31, mask, wordcount);
            break;
        case PSHIFT_PSRLQ:
            if (imm >= 64)
                mask = 0;
            cpu_psrlq((LARGE*) dest, imm & 63, mask, wordcount);
            break;
        case PSHIFT_PSRLDQ:
            if (imm >= 128)
                mask = 0;
            cpu_psrldq((LARGE*) dest, imm & 127, mask);
            break;
        case PSHIFT_PSLLQ:
            if (imm >= 64)
                mask = 0;
            cpu_psllq((LARGE*) dest, imm & 63, mask, wordcount);
            break;
        case PSHIFT_PSLLDQ:
            if (imm >= 128)
                mask = 0;
            cpu_pslldq((LARGE*) dest, imm & 127, mask);
            break;
    }
}

int CpuInternal::execute_0F70_76(struct decoded_instruction* i) {
    DWORD* dest32, flags = i->flags;
    int imm;
    switch (i->imm8 & 15) {
        case PSHUFW_MGqMEqIb:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            imm = i->imm16 >> 8;
            pshuf(dest32, m_result_ptr, imm, 1);
            break;
        case PSHUFHW_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            imm = i->imm16 >> 8;
            pshuf(dest32 + 2, m_result_ptr + 8, imm, 1);
            break;
        case PSHUFLW_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[2] = m_result_ptr.get<DWORD>(8);
            dest32[3] = m_result_ptr.get<DWORD>(12);
            imm = i->imm16 >> 8;
            pshuf(dest32, m_result_ptr, imm, 1);
            break;
        case PSHUFD_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            imm = i->imm16 >> 8;
            pshuf(dest32, m_result_ptr, imm, 2);
            break;
        case PSHIFT_MGqIb:

            if (cpu_mmx_check())
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 8 & 15);
            imm = i->imm16 >> 8;
            pshift(dest32, i->imm8 >> 4 & 15, 4, imm);
            break;
        case PSHIFT_XEoIb:
            if (cpu_sse_exception())
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 8 & 15);
            imm = i->imm16 >> 8;
            pshift(dest32, i->imm8 >> 4 & 15, 8, imm);
            break;
        case PCMPEQB_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pcmpeqb((BYTE*) dest32, 8);
            break;
        case PCMPEQB_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pcmpeqb((BYTE*) dest32, 16);
            break;
        case PCMPEQW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pcmpeqw((WORD*) dest32, 4);
            break;
        case PCMPEQW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pcmpeqw((WORD*) dest32, 8);
            break;
        case PCMPEQD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pcmpeqd(dest32, 2);
            break;
        case PCMPEQD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pcmpeqd(dest32, 4);
            break;
    }
    return 0;
}

int CpuInternal::execute_0F60_67(struct decoded_instruction* i) {
    DWORD* dest32, flags = i->flags;
    if (i->imm8 & 1) {
        if (cpu_sse_exception())
            return 1;
    } else {

        if (cpu_mmx_check())
            return 1;
    }
    switch (i->imm8 & 15) {
        case PUNPCKLBW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 8, 1);
            break;
        case PUNPCKLBW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 16, 1);
            break;
        case PUNPCKLWD_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 8, 2);
            break;
        case PUNPCKLWD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 16, 2);
            break;
        case PUNPCKLDQ_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 8, 4);
            break;
        case PUNPCKLDQ_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            punpckl(dest32, 16, 4);
            break;
        case PACKSSWB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            packsswb(dest32, 4);
            break;
        case PACKSSWB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            packsswb(dest32, 8);
            break;
        case PCMPGTB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pcmpgtb((int8_t*) dest32, 8);
            break;
        case PCMPGTB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pcmpgtb((int8_t*) dest32, 16);
            break;
        case PCMPGTW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pcmpgtw((int16_t*) dest32, 4);
            break;
        case PCMPGTW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pcmpgtw((int16_t*) dest32, 8);
            break;
        case PCMPGTD_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pcmpgtd((int32_t*) dest32, 2);
            break;
        case PCMPGTD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pcmpgtd((int32_t*) dest32, 4);
            break;
        case PACKUSWB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            packuswb(dest32, 4);
            break;
        case PACKUSWB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            packuswb(dest32, 8);
            break;
    }
    return 0;
}

DWORD CpuInternal::get_shift(int bytes) {
    for (int i = 1; i < bytes; i++)
        if (m_result_ptr.get<BYTE>(i))
            return 0xFF;
    return m_result_ptr.get<BYTE>(0);
}

int CpuInternal::execute_0FD0_D7(struct decoded_instruction* i) {
    DWORD* dest32, * src32, flags = i->flags;
    switch (i->imm8 & 15) {
        case PSRLW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRLW, 4, get_shift(8));
            break;
        case PSRLW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRLW, 8, get_shift(8));
            break;
        case PSRLD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRLD, 4, get_shift(8));
            break;
        case PSRLD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRLD, 8, get_shift(8));
            break;
        case PSRLQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRLQ, 4, get_shift(8));
            break;
        case PSRLQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRLQ, 8, get_shift(8));
            break;
        case PADDQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            *((LARGE*) dest32) += m_result_ptr.get<LARGE>();
            break;
        case PADDQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            *((LARGE*) (dest32)) += m_result_ptr.get<LARGE>();
            *((LARGE*) (&dest32[2])) += m_result_ptr.get<LARGE>(8);
            break;
        case PMULLW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmullw((WORD*) dest32, 4, 0);
            break;
        case PMULLW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmullw((WORD*) dest32, 8, 0);
            break;
        case MOVQ_XEqXGq:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_write_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            if ((flags & (1 << 25))) {
                m_result_ptr.set<DWORD>(0, 8);
                m_result_ptr.set<DWORD>(0, 12);
            }
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVQ2DQ_XGoMEq:

            if (cpu_mmx_check())
                return 1;
            if (cpu_sse_exception())
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            src32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] = src32[0];
            dest32[1] = src32[1];
            dest32[2] = 0;
            dest32[3] = 0;
            break;
        case MOVDQ2Q_MGqXEo:

            if (cpu_mmx_check())
                return 1;
            if (cpu_sse_exception())
                return 1;
            src32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] = src32[0];
            dest32[1] = src32[1];
            break;
        case PMOVMSKB_GdMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            m_reg32[flags >> 12 & 15] = pmovmskb(8);
            break;
        case PMOVMSKB_GdXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            m_reg32[flags >> 12 & 15] = pmovmskb(16);
            break;
    }
    return 0;
}

int CpuInternal::execute_0FD8_DF(struct decoded_instruction* i) {
    DWORD* dest32, flags = i->flags;
    if (i->imm8 & 1) {
        if (cpu_sse_exception())
            return 1;
    } else {
        if (cpu_mmx_check())
            return 1;
    }
    switch (i->imm8 & 15) {
        case PSUBUSB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubusb((BYTE*) dest32, 8);
            break;
        case PSUBUSB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubusb((BYTE*) dest32, 16);
            break;
        case PSUBUSW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubusw((WORD*) dest32, 4);
            break;
        case PSUBUSW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubusw((WORD*) dest32, 8);
            break;
        case PMINUB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pminub((BYTE*) dest32, 8);
            break;
        case PMINUB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pminub((BYTE*) dest32, 16);
            break;
        case PAND_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] &= m_result_ptr.get<DWORD>();
            dest32[1] &= m_result_ptr.get<DWORD>(4);
            break;
        case PAND_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] &= m_result_ptr.get<DWORD>();
            dest32[1] &= m_result_ptr.get<DWORD>(4);
            dest32[2] &= m_result_ptr.get<DWORD>(8);
            dest32[3] &= m_result_ptr.get<DWORD>(12);
            break;
        case PADDUSB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddusb((BYTE*) dest32, 8);
            break;
        case PADDUSB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddusb((BYTE*) dest32, 16);
            break;
        case PADDUSW_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddusw((WORD*) dest32, 4);
            break;
        case PADDUSW_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddusw((WORD*) dest32, 8);
            break;
        case PMAXUB_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmaxub((BYTE*) dest32, 8);
            break;
        case PMAXUB_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmaxub((BYTE*) dest32, 16);
            break;
        case PANDN_MGqMEq:
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest32[0] = ~dest32[0] & m_result_ptr.get<DWORD>();
            dest32[1] = ~dest32[1] & m_result_ptr.get<DWORD>(4);
            break;
        case PANDN_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = ~dest32[0] & m_result_ptr.get<DWORD>();
            dest32[1] = ~dest32[1] & m_result_ptr.get<DWORD>(4);
            dest32[2] = ~dest32[2] & m_result_ptr.get<DWORD>(8);
            dest32[3] = ~dest32[3] & m_result_ptr.get<DWORD>(12);
            break;
    }
    return 0;
}

int CpuInternal::execute_0F7E_7F(struct decoded_instruction* i) {
    DWORD* dest32, flags = i->flags;
    switch (i->imm8 & 7) {
        case MOVD_EdMGd:

            if (cpu_mmx_check())
                return 1;
            if ((get_reg_write_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVD_EdXGd:
            if (cpu_sse_exception())
                return 1;
            if ((get_reg_write_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVQ_XGqXEq:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = m_result_ptr.get<DWORD>();
            dest32[1] = m_result_ptr.get<DWORD>(4);
            dest32[2] = 0;
            dest32[3] = 0;
            break;
        case MOVQ_MEqMGq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_write_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_src(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVDQA_XEqXGq:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_write_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            m_result_ptr.set<DWORD>(dest32[2], 8);
            m_result_ptr.set<DWORD>(dest32[3], 12);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVDQU_XEqXGq:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_write_ptr(flags, i, 4, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            m_result_ptr.set<DWORD>(dest32[2], 8);
            m_result_ptr.set<DWORD>(dest32[3], 12);
            if (m_write_back && write_back_handler())
                return 1;
            break;
    }
    return 0;
}

int CpuInternal::execute_0FF8_FE(struct decoded_instruction* i) {
    DWORD flags = i->flags;
    void* dest;
    switch (i->imm8 & 15) {
        case PSUBB_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubb((BYTE*) dest, 8);
            break;
        case PSUBB_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubb((BYTE*) dest, 16);
            break;
        case PSUBW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubw((WORD*) dest, 4);
            break;
        case PSUBW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubw((WORD*) dest, 8);
            break;
        case PSUBD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubd((DWORD*) dest, 2);
            break;
        case PSUBD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubd((DWORD*) dest, 4);
            break;
        case PSUBQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psubq((LARGE*) dest, 1);
            break;
        case PSUBQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psubq((LARGE*) dest, 2);
            break;
        case PADDB_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddb((BYTE*) dest, 8);
            break;
        case PADDB_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddb((BYTE*) dest, 16);
            break;
        case PADDW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddw((WORD*) dest, 4);
            break;
        case PADDW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddw((WORD*) dest, 8);
            break;
        case PADDD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            paddd((DWORD*) dest, 2);
            break;
        case PADDD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            paddd((DWORD*) dest, 4);
            break;
    }
    return 0;
}

int CpuInternal::execute_0FC2_C6(struct decoded_instruction* i) {
    DWORD flags = i->flags, * dest32;
    WORD op, * dest16;
    int imm = i->imm16 >> 8, fp_exception = 0;
    switch (i->imm8 & 15) {
        case CMPPS_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = cmpps(dest32[0], m_result_ptr.get<float32>(), imm);
            dest32[1] = cmpps(dest32[1], m_result_ptr.get<float32>(4), imm);
            dest32[2] = cmpps(dest32[2], m_result_ptr.get<float32>(8), imm);
            dest32[3] = cmpps(dest32[3], m_result_ptr.get<float32>(12), imm);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CMPSS_XGdXEdIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = cmpps(dest32[0], m_result_ptr.get<float32>(), imm);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CMPPD_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = dest32[1] = cmppd(*(float64*) (&dest32[0]), m_result_ptr.get<float64>(), imm);
            dest32[2] = dest32[3] = cmppd(*(float64*) (&dest32[2]), m_result_ptr.get<float64>(8), imm);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CMPSD_XGqXEqIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = dest32[1] = cmppd(*(float64*) (&dest32[0]), m_result_ptr.get<float64>(), imm);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MOVNTI_EdGd:
            if ((get_reg_write_ptr(flags, i)))
                return 1;
            dest32 = (DWORD*) get_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(*dest32);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case PINSRW_MGqEdIb:
            if (cpu_sse_exception())
                return 1;
            if ((flags & (1 << 25)))
                op = m_reg32[flags >> 8 & 15];
            else {
                DWORD addr_ = cpu_get_linaddr2(flags, i), shift_ = m_tlb_shift_read,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        op = m_read_result;
                    else
                        return 1;
                } else {
                    op = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }

            dest16 = (WORD*) get_mmx_reg_dest(flags >> 12 & 15);
            dest16[imm & 3] = op;
            break;
        case PINSRW_XGoEdIb:
            if (cpu_sse_exception())
                return 1;
            if ((flags & (1 << 25)))
                op = m_reg32[flags >> 8 & 15];
            else {
                DWORD addr_ = cpu_get_linaddr2(flags, i), shift_ = m_tlb_shift_read,
                        tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        op = m_read_result;
                    else
                        return 1;
                } else {
                    op = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }

            dest16 = (WORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest16[imm & 7] = op;
            break;
        case PEXTRW_GdMEqIb:

            if (cpu_mmx_check())
                return 1;
            dest16 = (WORD*) get_mmx_reg_dest(flags >> 8 & 15);
            m_reg32[flags >> 12 & 15] = dest16[imm & 3];
            break;
        case PEXTRW_GdXEoIb:
            if (cpu_sse_exception())
                return 1;
            dest16 = (WORD*) get_sse_reg_dest(flags >> 8 & 15);
            m_reg32[flags >> 12 & 15] = dest16[imm & 7];
            break;
        case SHUFPS_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            shufps(dest32, imm);
            break;
        case SHUFPD_XGoXEoIb:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            shufpd(dest32, imm);
            break;
    }
    return fp_exception;
}

int CpuInternal::execute_0F58_5F(struct decoded_instruction* i) {
    if (cpu_sse_exception())
        return 1;
    DWORD flags = i->flags, * dest32;
    float64* dest64;
    int fp_exception = 0;
    switch (i->imm8 & 31) {
        case ADDPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_add(dest32[0], m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_add(dest32[1], m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_add(dest32[2], m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_add(dest32[3], m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case ADDSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_add(dest32[0], m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case ADDPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_add(dest64[0], m_result_ptr.get<float64>(), &m_status);
            dest64[1] = float64_add(dest64[1], m_result_ptr.get<float64>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case ADDSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_add(dest64[0], m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MULPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_mul(dest32[0], m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_mul(dest32[1], m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_mul(dest32[2], m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_mul(dest32[3], m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MULSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 0)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_mul(dest32[0], m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MULPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_mul(dest64[0], m_result_ptr.get<float64>(), &m_status);
            dest64[1] = float64_mul(dest64[1], m_result_ptr.get<float64>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MULSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_mul(dest64[0], m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTPS2PD_XGoXEo: {
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            float32 tmp = m_result_ptr.get<float32>(4);
            dest64[0] = float32_to_float64(m_result_ptr.get<float32>(), &m_status);
            dest64[1] = float32_to_float64(tmp, &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        }
        case CVTPD2PS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float64_to_float32(m_result_ptr.get<float64>(), &m_status);
            dest32[1] = float64_to_float32(m_result_ptr.get<float64>(8), &m_status);
            dest32[2] = 0;
            dest32[3] = 0;
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTSS2SD_XGoXEd:
            if ((get_sse_read_ptr(flags, i, 1, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float32_to_float64(m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTSD2SS_XGoXEq:
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float64_to_float32(m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTDQ2PS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = int32_to_float32(m_result_ptr.get<int32_t>(), &m_status);
            dest32[1] = int32_to_float32(m_result_ptr.get<int32_t>(4), &m_status);
            dest32[2] = int32_to_float32(m_result_ptr.get<int32_t>(8), &m_status);
            dest32[3] = int32_to_float32(m_result_ptr.get<int32_t>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTPS2DQ_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_to_int32(m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_to_int32(m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_to_int32(m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_to_int32(m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTTPS2DQ_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_to_int32_round_to_zero(m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_to_int32_round_to_zero(m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_to_int32_round_to_zero(m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_to_int32_round_to_zero(m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SUBPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_sub(dest32[0], m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_sub(dest32[1], m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_sub(dest32[2], m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_sub(dest32[3], m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SUBSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_sub(dest32[0], m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SUBPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_sub(dest64[0], m_result_ptr.get<float64>(), &m_status);
            dest64[1] = float64_sub(dest64[1], m_result_ptr.get<float64>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case SUBSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_sub(dest64[0], m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MINPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_min(dest32[0], m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_min(dest32[1], m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_min(dest32[2], m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_min(dest32[3], m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MINSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_min(dest32[0], m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MINPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_min(dest64[0], m_result_ptr.get<float64>(), &m_status);
            dest64[1] = float64_min(dest64[1], m_result_ptr.get<float64>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MINSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_min(dest64[0], m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case DIVPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_div(dest32[0], m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_div(dest32[1], m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_div(dest32[2], m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_div(dest32[3], m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case DIVSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_div(dest32[0], m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case DIVPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_div(dest64[0], m_result_ptr.get<float64>(), &m_status);
            dest64[1] = float64_div(dest64[1], m_result_ptr.get<float64>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case DIVSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_div(dest64[0], m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MAXPS_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_max(dest32[0], m_result_ptr.get<float32>(), &m_status);
            dest32[1] = float32_max(dest32[1], m_result_ptr.get<float32>(4), &m_status);
            dest32[2] = float32_max(dest32[2], m_result_ptr.get<float32>(8), &m_status);
            dest32[3] = float32_max(dest32[3], m_result_ptr.get<float32>(12), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MAXSS_XGdXEd:
            if ((get_sse_read_ptr(flags, i, 1, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float32_max(dest32[0], m_result_ptr.get<float32>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MAXPD_XGoXEo:
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_max(dest64[0], m_result_ptr.get<float64>(), &m_status);
            dest64[1] = float64_max(dest64[1], m_result_ptr.get<float64>(8), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case MAXSD_XGqXEq:
            if ((get_sse_read_ptr(flags, i, 2, 0)))
                return 1;
            dest64 = (float64*) get_sse_reg_dest(flags >> 12 & 15);
            dest64[0] = float64_max(dest64[0], m_result_ptr.get<float64>(), &m_status);
            fp_exception = cpu_sse_handle_exceptions();
            break;
    }
    return fp_exception;
}

int CpuInternal::execute_0FE0_E7(struct decoded_instruction* i) {
    DWORD flags = i->flags, * dest32;
    int fp_exception = 0;
    switch (i->imm8 & 31) {
        case PAVGB_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pavgb(dest32, 8);
            break;
        case PAVGB_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pavgb(dest32, 16);
            break;
        case PSRAW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRAW, 4, get_shift(8));
            break;
        case PSRAD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRAD, 4, get_shift(8));
            break;
        case PSRAW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRAW, 8, get_shift(16));
            break;
        case PSRAD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSRAD, 8, get_shift(16));
            break;
        case PAVGW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pavgw(dest32, 4);
            break;
        case PAVGW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pavgw(dest32, 8);
            break;
        case PMULHUW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmuluw(dest32, 4, 16);
            break;
        case PMULHUW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmuluw(dest32, 8, 16);
            break;
        case PMULHW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmullw((WORD*) dest32, 4, 16);
            break;
        case PMULHW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmullw((WORD*) dest32, 8, 16);
            break;
        case CVTPD2DQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float64_to_int32(m_result_ptr.get<float64>(), &m_status);
            dest32[1] = float64_to_int32(m_result_ptr.get<float64>(8), &m_status);
            dest32[2] = 0;
            dest32[3] = 0;
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTTPD2DQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            dest32[0] = float64_to_int32_round_to_zero(m_result_ptr.get<float64>(), &m_status);
            dest32[1] = float64_to_int32_round_to_zero(m_result_ptr.get<float64>(8), &m_status);
            dest32[2] = 0;
            dest32[3] = 0;
            fp_exception = cpu_sse_handle_exceptions();
            break;
        case CVTDQ2PD_XGoXEq: {
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            DWORD dword1 = m_result_ptr.get<DWORD>(), dword2 = m_result_ptr.get<DWORD>(4);
            *(LARGE*) (&dest32[0]) = int32_to_float64(dword1);
            *(LARGE*) (&dest32[2]) = int32_to_float64(dword2);
            fp_exception = cpu_sse_handle_exceptions();
            break;
        }
        case MOVNTQ_MEqMGq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_write_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_src(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            if (m_write_back && write_back_handler())
                return 1;
            break;
        case MOVNTDQ_XEoXGo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_write_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            m_result_ptr.set<DWORD>(dest32[0]);
            m_result_ptr.set<DWORD>(dest32[1], 4);
            m_result_ptr.set<DWORD>(dest32[2], 8);
            m_result_ptr.set<DWORD>(dest32[3], 12);
            if (m_write_back && write_back_handler())
                return 1;
            break;
    }
    return fp_exception;
}

int CpuInternal::execute_0FF1_F7(struct decoded_instruction* i) {
    DWORD flags = i->flags, * dest32, linaddr;
    BYTE* mask, * src8;
    switch (i->imm8) {
        case PSLLW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_src(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSLLW, 4, get_shift(8));
            break;
        case PSLLW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSLLW, 8, get_shift(8));
            break;
        case PSLLD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_src(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSLLD, 4, get_shift(8));
            break;
        case PSLLD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSLLD, 8, get_shift(8));
            break;
        case PSLLQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_src(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSLLQ, 4, get_shift(8));
            break;
        case PSLLQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 2, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pshift(dest32, PSHIFT_PSLLQ, 8, get_shift(8));
            break;
        case PMULLUDQ_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmuludq(dest32, 2);
            break;
        case PMULLUDQ_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmuludq(dest32, 4);
            break;
        case PMADDWD_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            pmaddwd(dest32, 2);
            break;
        case PMADDWD_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            pmaddwd(dest32, 4);
            break;
        case PSADBW_MGqMEq:

            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            dest32 = (DWORD*) get_mmx_reg_dest(flags >> 12 & 15);
            psadbw(dest32, 1);
            break;
        case PSADBW_XGoXEo:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 1)))
                return 1;
            dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
            psadbw(dest32, 2);
            break;
        case MASKMOVQ_MEqMGq:

            if (cpu_mmx_check())
                return 1;
            linaddr = m_reg32[7] + m_seg_base[flags >> 22 & 7];
            src8 = (BYTE*) get_mmx_reg_src(flags >> 12 & 15);
            mask = (BYTE*) get_mmx_reg_src(flags >> 8 & 15);

            for (int i = 0; i < 8; i++) {
                if (mask[i] & 0x80) {
                    DWORD addr_ = linaddr + i, shift_ = m_tlb_shift_write, data_ = src8[i],
                            tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
            break;
        case MASKMOVDQ_XEoXGo:
            if (cpu_sse_exception())
                return 1;
            linaddr = m_reg32[7] + m_seg_base[flags >> 22 & 7];
            src8 = (BYTE*) get_sse_reg_dest(flags >> 12 & 15);
            mask = (BYTE*) get_sse_reg_dest(flags >> 8 & 15);
            for (int i = 0; i < 16; i++) {
                if (mask[i] & 0x80) {
                    DWORD addr_ = linaddr + i, shift_ = m_tlb_shift_write, data_ = src8[i],
                            tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
            break;
    }
    return 0;
}

int CpuInternal::cpu_emms(void) {
    if (cpu_mmx_check())
        return 1;
    m_fpu.tag_word = 0xFFFF;
    return 0;
}

int CpuInternal::execute_0F7C_7D(struct decoded_instruction* i) {
    DWORD flags = i->flags, * dest32;
    union
    {
        DWORD b32[4];
        LARGE b64[2];
    } tmp;

    if (cpu_sse_exception())
        return 1;
    if ((get_sse_read_ptr(flags, i, 4, 1)))
        return 1;
    dest32 = (DWORD*) get_sse_reg_dest(flags >> 12 & 15);
    switch (i->imm8 & 3) {
        case HADDPD_XGoXEo:
            tmp.b64[0] = float64_add(*(float64*) (&dest32[0]), *(float64*) (&dest32[2]), &m_status);
            tmp.b64[1] = float64_add(m_result_ptr.get<float64>(), m_result_ptr.get<float64>(8), &m_status);
            break;
        case HADDPS_XGoXEo:
            tmp.b32[0] = float32_add(dest32[0], dest32[1], &m_status);
            tmp.b32[1] = float32_add(dest32[2], dest32[3], &m_status);
            tmp.b32[2] = float32_add(m_result_ptr.get<float32>(), m_result_ptr.get<float32>(4), &m_status);
            tmp.b32[3] = float32_add(m_result_ptr.get<float32>(8), m_result_ptr.get<float32>(12), &m_status);
            break;
        case HSUBPD_XGoXEo:
            tmp.b64[0] = float64_sub(*(float64*) (&dest32[0]), *(float64*) (&dest32[2]), &m_status);
            tmp.b64[1] = float64_sub(m_result_ptr.get<float64>(), m_result_ptr.get<float64>(8), &m_status);
            break;
        case HSUBPS_XGoXEo:
            tmp.b32[0] = float32_sub(dest32[0], dest32[1], &m_status);
            tmp.b32[1] = float32_sub(dest32[2], dest32[3], &m_status);
            tmp.b32[2] = float32_sub(m_result_ptr.get<float32>(), m_result_ptr.get<float32>(4), &m_status);
            tmp.b32[3] = float32_sub(m_result_ptr.get<float32>(8), m_result_ptr.get<float32>(12), &m_status);
            break;
    }
    memcpy(dest32, tmp.b32, 16);
    return cpu_sse_handle_exceptions();
}

int CpuInternal::execute_0F38(struct decoded_instruction* i) {
    DWORD flags = i->flags;
    switch (i->imm8) {
        case 0:
            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            pshufb(get_mmx_reg_dest(flags >> 12 & 15), 8);
            break;
        case 0x1C:
            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            pabsb(get_mmx_reg_dest(flags >> 12 & 15), 8);
            break;
        case 0x1D:
            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            pabsw(get_mmx_reg_dest(flags >> 12 & 15), 4);
            break;
        case 0x1E:
            if (cpu_mmx_check())
                return 1;
            if ((get_mmx_read_ptr(flags, i, 2)))
                return 1;
            pabsd(get_mmx_reg_dest(flags >> 12 & 15), 2);
            break;
        default:
            util_abort();
            break;
    }
    return 0;
}

int CpuInternal::execute_660F38(struct decoded_instruction* i) {
    DWORD flags = i->flags;
    switch (i->imm8) {
        case 0:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            pshufb(get_sse_reg_dest(flags >> 12 & 15), 16);
            break;
        case 0x1C:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            pabsb(get_sse_reg_dest(flags >> 12 & 15), 16);
            break;
        case 0x1D:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            pabsw(get_sse_reg_dest(flags >> 12 & 15), 8);
            break;
        case 0x1E:
            if (cpu_sse_exception())
                return 1;
            if ((get_sse_read_ptr(flags, i, 4, 0)))
                return 1;
            pabsd(get_sse_reg_dest(flags >> 12 & 15), 4);
            break;
        default:
            util_abort();
            break;
    }
    return 0;
}
