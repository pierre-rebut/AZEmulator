#include "cpu.h"

int CpuInternal::cpu_set_cpuid(int cpuid)
{
    m_winnt_limit_cpuid = cpuid;
    return 0;
}
void CpuInternal::cpuid(void)
{
    switch (m_reg32[0]) {
        case 0:
            m_reg32[0] = 10;
            m_reg32[1] = 0x6c65746e;
            m_reg32[2] = 0x49656e69;
            m_reg32[3] = 0x756e6547;
            if (m_winnt_limit_cpuid)
                m_reg32[0] = 2;
            break;
        case 1:
            m_reg32[0] = 0x000106C2;
            m_reg32[1] = 0x40C39D;
            m_reg32[2] = 0xBFEBF9FF | cpu_apic_connected() << 9;
            m_reg32[3] = 0x00010800;
            break;
        case 2:
            m_reg32[0] = 0x4fba5901;
            m_reg32[1] = 0;
            m_reg32[2] = 0;
            m_reg32[3] = 0x0e3080c0;
            break;
        case 4:
            switch (m_reg32[1]) {
                case 0:
                    m_reg32[0] = 0x00004121;
                    m_reg32[3] = 0x0140003f;
                    m_reg32[1] = 0x0000003F;
                    m_reg32[2] = 0x00000001;
                    break;
                case 1:
                    m_reg32[0] = 0x00004122;
                    m_reg32[3] = 0x01C0003f;
                    m_reg32[1] = 0x0000003F;
                    m_reg32[2] = 0x00000001;
                    break;
                case 2:
                    m_reg32[0] = 0x00004143;
                    m_reg32[3] = 0x01C0003F;
                    m_reg32[1] = 0x000003FF;
                    m_reg32[2] = 0x00000001;
                    break;
                default:
                    m_reg32[0] = 0;
                    m_reg32[3] = 0;
                    m_reg32[1] = 0;
                    m_reg32[2] = 0;
                    return;
            }
            break;
        case 5:
            m_reg32[0] = 0x00000040;
            m_reg32[1] = 0x00000003;
            m_reg32[2] = 0x00020220;
            m_reg32[3] = 0x00000040;
            break;
        case 6:
            m_reg32[0] = 1;
            m_reg32[1] = 1;
            m_reg32[2] = 0;
            m_reg32[3] = 2;
            break;
        case 0x40000000:
        case 0x40000001:
        case 10:
            m_reg32[0] = 0x07280203;
            m_reg32[3] = 0x00000000;
            m_reg32[1] = 0x00000000;
            m_reg32[2] = 0x00002501;
            break;
        case 0x80000000:
            m_reg32[0] = 0x80000008;
            m_reg32[1] = m_reg32[2] = m_reg32[3] = 0;
            break;
        case 0x80000001:
            m_reg32[0] = 0;
            m_reg32[1] = 1;
            m_reg32[2] = m_reg32[3] = 0;
            break;
        case 0x80000002 ... 0x80000004: {
            static const char *brand_string = "         Intel(R) Atom(TM) CPU N270   @ 1.60GHz";
            static const int   reg_ids[]    = {0, 3, 1, 2};
            int                offset       = (m_reg32[0] - 0x80000002) << 4;
            for (int i = 0; i < 16; i++) {
                int shift = (i & 3) << 3, reg = reg_ids[i >> 2];
                m_reg32[reg] &= ~(0xFF << shift);
                m_reg32[reg] |= brand_string[offset + i] << shift;
            }
            break;
        }
        case 0x80000005:
            m_reg32[0] = 0x01ff01ff;
            m_reg32[1] = 0x40020140;
            m_reg32[3] = 0x01ff01ff;
            m_reg32[2] = 0x40020140;
            break;
        case 0x80000006:
            m_reg32[0] = 0;
            m_reg32[1] = 0x02008140;
            m_reg32[3] = 0x42004200;
            m_reg32[2] = 0;
            break;
        case 0x80000008:
            m_reg32[0] = 0x2028;
            m_reg32[1] = m_reg32[2] = m_reg32[3] = 0;
            break;
        default:
            goto __annoying_gcc_workaround;
        case 0x80860000 ... 0x80860007:
        __annoying_gcc_workaround:
            m_reg32[0] = 0;
            m_reg32[1] = m_reg32[2] = m_reg32[3] = 0;
            break;
    }
}
int CpuInternal::rdmsr(DWORD index, DWORD *high, DWORD *low)
{
    LARGE value;
    switch (index) {
        case 0x1B:
            if (!cpu_apic_connected()) {
                    cpu_exception(13, (0) | 0x10000);
                    return 1;
                }
            value = m_apic_base;
            break;
        case 0x250 ... 0x26F:
            value = m_mtrr_fixed[index - 0x250];
            break;
        case 0x200 ... 0x20F:
            value = m_mtrr_variable_addr_mask[index ^ 0x200];
            break;
        case 0x277:
            value = m_page_attribute_tables;
            break;
        case 0x2FF:
            value = m_mtrr_deftype;
            break;
        default:
            value = 0;
            break;
        case 0x174 ... 0x176:
            value = m_sysenter[index - 0x174];
            break;
        case 0xFE:
            value = 0x508;
            break;
        case 0x10:
            value = cpu_get_cycles() - m_tsc_fudge;
            break;
        case 0xc0000080:
            value = m_ia32_efer;
    }
    *high = value >> 32;
    *low  = value & 0xFFFFFFFF;
    return 0;
}
int CpuInternal::wrmsr(DWORD index, DWORD high, DWORD low)
{
    LARGE msr_value = ((LARGE)high) << 32 | (LARGE)low;
    switch (index) {
        case 0x1B:
            m_apic_base = msr_value;
            break;
        case 0x174 ... 0x176:
            m_sysenter[index - 0x174] = low;
            break;
        case 0x8B:
        case 0x17:
        case 0xC1:
        case 0xC2:
        case 0x179:
        case 0x17A:
        case 0x17B:
        case 0x186:
        case 0x187:
        case 0x19A:
        case 0x19B:
        case 0xFE:
            break;
        case 0x250 ... 0x26F:
            m_mtrr_fixed[index - 0x250] = msr_value;
            break;
        case 0x200 ... 0x20F:
            m_mtrr_variable_addr_mask[index ^ 0x200] = msr_value;
            break;
        case 0x277:
            m_page_attribute_tables = msr_value;
            break;
        case 0x2FF:
            m_mtrr_deftype = msr_value;
            break;
        case 0x10:
            m_tsc_fudge = cpu_get_cycles() - msr_value;
            break;
        case 0xc0000080:
            m_ia32_efer = msr_value;
            break;
        default: {
        }
    }
    return 0;
}
int CpuInternal::pushf(void)
{
    if ((m_eflags & 0x20000 && (m_eflags >> 12 & 3) < 3)) {
        if ((m_cr[4] & (1 << 0)) != 0) {
            WORD flags = cpu_get_eflags();
            flags &= ~(1 << 9);
            flags |= ((m_eflags & (1 << 19)) != 0) << 9;
            flags |= 0x3000;
            return cpu_push16(flags);
        } else {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }
    }
    return cpu_push16(cpu_get_eflags() & 0xFFFF);
}
int CpuInternal::pushfd(void)
{
    if ((m_eflags & 0x20000 && (m_eflags >> 12 & 3) < 3)) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
    }

    return cpu_push32(cpu_get_eflags() & 0x00FCFFFF);
}
int CpuInternal::popf(void)
{
    if (m_eflags & 0x20000) {
        if ((m_eflags >> 12 & 3) == 3)
            goto cpl_gt_0;
        else {
            if (m_cr[4] & (1 << 0)) {
                WORD temp_flags;
                if (cpu_pop16(&temp_flags))
                    return 1;
                if (!((m_eflags & 0x100000 && temp_flags & (1 << 9)) || (temp_flags & (1 << 8)))) {
                    m_eflags &= ~0x80000;
                    m_eflags |= (temp_flags & (1 << 9)) ? 0x80000 : 0;
                    const DWORD flags_mask = 0xFFFF ^ (0x200 | 0x3000);
                    cpu_set_eflags((temp_flags & flags_mask) | (m_eflags & ~flags_mask));
                    return 0;
                }
            }
            {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }
        }
    }
    WORD eflags;
    if (m_cpl == 0 || (m_cr[0] & 1) == 0) {
        if (cpu_pop16(&eflags))
            return 1;
        cpu_set_eflags(eflags | (m_eflags & 0xFFFF0000));
    } else {
    cpl_gt_0:
        if (cpu_pop16(&eflags))
            return 1;
        cpu_set_eflags((eflags & ~0x3000) | (m_eflags & (0xFFFF0000 | 0x3000)));
    }
    return 0;
}
int CpuInternal::popfd(void)
{
    DWORD eflags;
    if (m_eflags & 0x20000) {
        if ((m_eflags >> 12 & 3) == 3) {
            if (cpu_pop32(&eflags))
                return 1;
            eflags &= ~(0x3000 | 0x100000 | 0x80000 | 0x20000 | 0x10000);
            cpu_set_eflags(eflags | (m_eflags & (0x3000 | 0x100000 | 0x80000 | 0x20000 | 0x10000)));
        } else {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }
    } else {
        if (m_cpl == 0 || (m_cr[0] & 1) == 0) {
            if (cpu_pop32(&eflags))
                return 1;
            eflags                  = eflags & ~0x10000;
            const DWORD preserve = 0x100000 | 0x80000 | 0x20000;
            cpu_set_eflags((eflags & ~preserve) | (m_eflags & preserve));
        } else {
            if (cpu_pop32(&eflags))
                return 1;
            eflags            = eflags & ~0x10000;
            DWORD preserve = 0x3000 | 0x100000 | 0x80000 | 0x20000;
            if ((unsigned int)m_cpl > (m_eflags >> 12 & 3))
                preserve |= 0x200;
            cpu_set_eflags((eflags & ~preserve) | (m_eflags & preserve));
        }
    }
    return 0;
}
int CpuInternal::ltr(DWORD selector)
{
    DWORD        selector_offset = selector & 0xFFFC, tss_access, tss_addr;
    struct seg_desc tss_desc;
    if (selector_offset == 0) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }
    if (((selector & 4) != 0)) {
            cpu_exception(13, (selector_offset) | 0x10000);
            return 1;
        }
    if (cpu_seg_load_descriptor2(7, selector, &tss_desc, 13, selector_offset))
        return 1;
    tss_access = ((&tss_desc)->raw[1] >> 8 & 0xFFFF);
    if ((tss_access & 0x80) == 0) {
            cpu_exception(11, (selector_offset) | 0x10000);
            return 1;
        }
    tss_addr = cpu_seg_descriptor_address(7, selector);
    tss_desc.raw[1] |= 0x200;
    {
        DWORD addr_ = tss_addr + 4, shift_ = 2, data_ = tss_desc.raw[1], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> 2, 2))
                return 1;
        } else {
            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
        }
    }

    m_seg_base[6]   = cpu_seg_get_base(&tss_desc);
    m_seg_limit[6]  = cpu_seg_get_limit(&tss_desc);
    m_seg_access[6] = ((&tss_desc)->raw[1] >> 8 & 0xFFFF);
    m_seg[6]        = selector;
    return 0;
}
int CpuInternal::lldt(DWORD selector)
{
    DWORD        selector_offset = selector & 0xFFFC, ldt_access;
    struct seg_desc ldt_desc;
    if (selector_offset == 0) {
        m_seg_base[8]   = 0;
        m_seg_limit[8]  = 0;
        m_seg_access[8] = 0;
        m_seg[8]        = selector;
        return 0;
    }
    if (((selector_offset & 4) != 0)) {
            cpu_exception(13, (selector_offset) | 0x10000);
            return 1;
        }
    if (cpu_seg_load_descriptor2(7, selector, &ldt_desc, 13, selector_offset))
        return 1;
    ldt_access = ((&ldt_desc)->raw[1] >> 8 & 0xFFFF);
    if ((ldt_access & 0x80) == 0) {
            cpu_exception(11, (selector_offset) | 0x10000);
            return 1;
        }
    m_seg_base[8]   = cpu_seg_get_base(&ldt_desc);
    m_seg_limit[8]  = cpu_seg_get_limit(&ldt_desc);
    m_seg_access[8] = ((&ldt_desc)->raw[1] >> 8 & 0xFFFF);
    m_seg[8]        = selector;
    return 0;
}
DWORD CpuInternal::lar(WORD op1, DWORD op2)
{
    WORD        op_offset = op1 & 0xFFFC;
    struct seg_desc op_info;
    DWORD        op_access;
    if (!op_offset)
        goto invalid;
    if (cpu_seg_load_descriptor(op1, &op_info, -1, -1))
        goto invalid;
    op_access = ((&op_info)->raw[1] >> 8 & 0xFFFF);
    int dpl;
    switch (((op_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
        case 0:
        case INTERRUPT_GATE_286:
        case TRAP_GATE_286:
        case 8:
        case 10:
        case 13:
        case INTERRUPT_GATE_386:
        case TRAP_GATE_386:
            goto invalid;
        case 0x18 ... 0x1B:
            dpl = (op_access >> 5 & 3);
            if (dpl < m_cpl || dpl < (op1 & 3))
                goto invalid;
        case 0x1C ... 0x1F:
        default:
            cpu_set_zf(1);
            return op_info.raw[1] & 0xFFFF00;
    }
invalid:
    cpu_set_zf(0);
    return op2;
}
DWORD CpuInternal::lsl(WORD op, DWORD op2)
{
    if ((m_cr[0] & 1) == 0 || (m_eflags & 0x20000)) {
            cpu_exception(6, 0);
            return 1;
        }
    WORD op_offset = op & 0xFFFC;
    int      op_access;
    if (!op_offset)
        goto invalid;
    struct seg_desc op_info;
    if (cpu_seg_load_descriptor(op, &op_info, -1, -1))
        goto invalid;
    int dpl;
    op_access = ((&op_info)->raw[1] >> 8 & 0xFFFF);
    switch (((op_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
        case 0:
        case 4 ... 7:
        case 12 ... 15:
        case 0x1E:
            goto invalid;
        case 0x18 ... 0x1B:
            dpl = (op_access >> 5 & 3);
            if (dpl < m_cpl || dpl < (op & 3))
                goto invalid;
        default:
            cpu_set_zf(1);
            return cpu_seg_get_limit(&op_info);
    }
invalid:
    cpu_set_zf(0);
    return op2;
}
void CpuInternal::verify_segment_access(WORD sel, int write)
{
    WORD        sel_offset = sel & 0xFFFC;
    int             zf         = 0;
    struct seg_desc seg;
    if (sel_offset) {
        if (cpu_seg_load_descriptor(sel, &seg, -1, -1) == 0) {
            int access = ((&seg)->raw[1] >> 8 & 0xFFFF);
            int type   = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
            int dpl = (access >> 5 & 3), rpl = (sel & 3);
            zf = 1;
            if (write) {
                if (!(type == 0x12 || type == 0x13 || type == 0x16 || type == 0x17))
                    zf = 0;
                else {
                    if ((dpl < m_cpl) || (dpl < rpl))
                        zf = 0;
                }
            } else {
                if (type >= 0x10 && type <= 0x1B) {
                    if ((dpl < m_cpl) || (dpl < rpl))
                        zf = 0;
                }
            }
        }
    }
    cpu_set_zf(zf);
}
WORD CpuInternal::arpl(WORD ptr, WORD reg)
{
    reg &= 3;
    if ((ptr & 3) < reg) {
        ptr = (ptr & ~3) | reg;
        cpu_set_zf(1);
    } else
        cpu_set_zf(0);

    return ptr;
}
