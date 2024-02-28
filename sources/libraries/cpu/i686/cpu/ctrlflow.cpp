#include <vector>
#include "cpu.h"

int CpuInternal::tss_is_16(int type)
{
    return type == BUSY_TSS_286 || type == AVAILABLE_TSS_286;
}
int CpuInternal::load_tss_from_task_gate(DWORD *seg, struct seg_desc *info)
{
    int new_seg = cpu_seg_gate_target_segment(info), offset = new_seg & 0xFFFC;
    if (((new_seg & 4) != 0)) {
            cpu_exception(10, (offset) | 0x10000);
            return 1;
        }
    if (cpu_seg_load_descriptor2(7, new_seg, info, 13, offset))
        return 1;
    int access = ((info)->raw[1] >> 8 & 0xFFFF), type = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
    if (type != AVAILABLE_TSS_286 && type != AVAILABLE_TSS_386) {
            cpu_exception(13, (offset) | 0x10000);
            return 1;
        }
    if ((access & 0x80) == 0) {
            cpu_exception(11, (offset) | 0x10000);
            return 1;
        }
    *seg = new_seg;
    return 0;
}
int CpuInternal::get_tss_esp(int level, int *dest)
{
    int temp;
    if (tss_is_16(((m_seg_access[6]) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)))) {
        int addr = 2 + (level * 4);
        if ((unsigned int)(addr + 2) >= m_seg_limit[6]) {
                cpu_exception(10, (m_seg[6] & 0xFFFC) | 0x10000);
                return 1;
            }
        {
            DWORD addr_ = addr + m_seg_base[6], shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    temp = m_read_result;
                else
                    return 1;
            } else {
                temp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
    } else {
        int addr = 4 + (level * 8);
        if ((unsigned int)(addr + 4) >= m_seg_limit[6]) {
                cpu_exception(10, (m_seg[6] & 0xFFFC) | 0x10000);
                return 1;
            }
        {
            DWORD addr_ = addr + m_seg_base[6], shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> 0, 0))
                    temp = m_read_result;
                else
                    return 1;
            } else {
                temp = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
    }
    *dest = temp;
    return 0;
}
int CpuInternal::get_tss_ss(int level, int *dest)
{
    int temp;
    if (tss_is_16(((m_seg_access[6]) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)))) {
        int addr = 2 + (level * 4) + 2;
        if ((unsigned int)(addr + 2) >= m_seg_limit[6]) {
                cpu_exception(10, (m_seg[6] & 0xFFFC) | 0x10000);
                return 1;
            }
        {
            DWORD addr_ = addr + m_seg_base[6], shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    temp = m_read_result;
                else
                    return 1;
            } else {
                temp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
    } else {
        int addr = 4 + (level * 8) + 4;
        if ((unsigned int)(addr + 4) >= m_seg_limit[6]) {
                cpu_exception(10, (m_seg[6] & 0xFFFC) | 0x10000);
                return 1;
            }
        {
            DWORD addr_ = addr + m_seg_base[6], shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> 0, 0))
                    temp = m_read_result;
                else
                    return 1;
            } else {
                temp = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
    }
    *dest = temp & 0xFFFF;
    return 0;
}
int CpuInternal::do_task_switch(int sel, struct seg_desc *info, int type, int eip)
{
    static const int tss_limits[] = {43, 103};
    int              offset = sel & 0xFFFC, limit = cpu_seg_get_limit(info), base = cpu_seg_get_base(info),
        access = ((info)->raw[1] >> 8 & 0xFFFF), tss_type = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
    if (((sel & 4) != 0)) {
            cpu_exception(10, (offset) | 0x10000);
            return 1;
        }
    if (limit <= tss_limits[type >> 3 & 1]) {
            cpu_exception(10, (offset) | 0x10000);
            return 1;
        }
    int old_tr_type  = ((m_seg_access[6]) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)),
        old_tr_limit = tss_limits[old_tr_type >> 3 & 1];
    DWORD tr_base = m_seg_base[6], desc_tbl, old_eflags = cpu_get_eflags();
    if (cpu_access_verify(tr_base, tr_base + old_tr_limit, 0))
        return 1;
    if (cpu_access_verify(tr_base, tr_base + old_tr_limit, 2))
        return 1;
    if (type == TASK_JMP || type == TASK_IRET) {
        if (tr_base == (DWORD)-1)
            util_abort();
        DWORD segid = ((sel & 4) != 0) ? 8 : 7, addr = m_seg_base[segid] + ((m_seg[6] & ~7)) + 5;
        {
            DWORD addr_ = addr, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> 0, 0))
                    desc_tbl = m_read_result;
                else
                    return 1;
            } else {
                desc_tbl = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        }
         {
            DWORD addr_ = addr, shift_ = 2, data_ = desc_tbl, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        if (type == TASK_IRET)
            old_eflags &= ~0x4000;
    }
    if (tss_type == AVAILABLE_TSS_386 || tss_type == BUSY_TSS_386) {
        {
            DWORD addr_ = tr_base + 0x20, shift_ = 2, data_ = eip, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = tr_base + 0x24, shift_ = 2, data_ = old_eflags, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        for (int i = 0; i < 8; i++) {
                DWORD addr_ = tr_base + 0x28 + (i * 4), shift_ = 2, data_ = m_reg32[i],
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> 2, 2))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
        for (int i = 0; i < 6; i++) {
                DWORD addr_ = tr_base + 0x48 + (i * 4), shift_ = 2, data_ = m_seg[i], tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> 2, 2))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
    } else {
        {
            DWORD addr_ = tr_base + 0x0E, shift_ = 2, data_ = eip, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        {
            DWORD addr_ = tr_base + 0x10, shift_ = 2, data_ = old_eflags, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        for (int i = 0; i < 8; i++) {
                DWORD addr_ = tr_base + 0x12 + (i * 2), shift_ = 2, data_ = m_reg32[i],
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> 2, 2))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
        for (int i = 0; i < 4; i++) {
                DWORD addr_ = tr_base + 0x22 + (i * 2), shift_ = 2, data_ = m_seg[i], tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> 2, 2))
                        return 1;
                } else {
                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                }
            }
    }
    if (type == TASK_INT || type == TASK_CALL) {
            DWORD addr_ = tr_base, shift_ = 2, data_ = m_seg[6], tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
    DWORD cr3 = 0, eflags, reg32[8], seg[6], ldt;
    if (tss_type == AVAILABLE_TSS_386 || tss_type == BUSY_TSS_386) {
      {
            DWORD addr_ = base + 0x1C, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> 0, 0))
                    cr3 = m_read_result;
                else
                    return 1;
            } else {
                cr3 = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        {
            DWORD addr_ = base + 0x20, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> 0, 0))
                    eip = m_read_result;
                else
                    return 1;
            } else {
                eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        {
            DWORD addr_ = base + 0x24, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> 0, 0))
                    eflags = m_read_result;
                else
                    return 1;
            } else {
                eflags = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        for (int i = 0; i < 8; i++) {
                DWORD addr_ = base + 0x28 + (i * 4), shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> 0, 0))
                        reg32[i] = m_read_result;
                    else
                        return 1;
                } else {
                    reg32[i] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            }
        for (int i = 0; i < 6; i++) {
                DWORD addr_ = base + 0x48 + (i * 4), shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> 0, 0))
                        seg[i] = m_read_result;
                    else
                        return 1;
                } else {
                    seg[i] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
         {
            DWORD addr_ = base + 0x60, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> 0, 0))
                    ldt = m_read_result;
                else
                    return 1;
            } else {
                ldt = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
    } else {
         {
            DWORD addr_ = base + 0x0E, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    eip = m_read_result;
                else
                    return 1;
            } else {
                eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
         {
            DWORD addr_ = base + 0x10, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    eflags = m_read_result;
                else
                    return 1;
            } else {
                eflags = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        for (int i = 0; i < 8; i++) {
             {
                DWORD addr_ = base + 0x12 + (i * 2), shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> 0, 0))
                        reg32[i] = m_read_result;
                    else
                        return 1;
                } else {
                    reg32[i] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
            reg32[i] |= 0xFFFF0000;
        }
        for (int i = 0; i < 4; i++) {
                DWORD addr_ = base + 0x22 + (i * 2), shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> 0, 0))
                        seg[i] = m_read_result;
                    else
                        return 1;
                } else {
                    seg[i] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                }
            }
         {
            DWORD addr_ = base + 0x2A, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    ldt = m_read_result;
                else
                    return 1;
            } else {
                ldt = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        seg[4] = 0;
        seg[5] = 0;
    }
    tr_base = cpu_seg_descriptor_address(7, sel);
    if (type == TASK_JMP || type == TASK_IRET) {
        if (tr_base == (DWORD)-1)
            util_abort();
        DWORD segid = ((sel & 4) != 0) ? 8 : 7, addr = m_seg_base[segid] + ((sel & ~7)) + 5;
         {
            DWORD addr_ = addr, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> 0, 0))
                    desc_tbl = m_read_result;
                else
                    return 1;
            } else {
                desc_tbl = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        }
        desc_tbl |= 2;
         {
            DWORD addr_ = addr, shift_ = 2, data_ = desc_tbl, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> 2, 2))
                    return 1;
            } else {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
    }
    m_cr[0] |= 8;
    m_seg[6]        = sel;
    m_seg_base[6]   = base;
    m_seg_limit[6]  = limit;
    m_seg_access[6] = access & ~2;
    m_seg_valid[6]  = 1 | 2;
    if (tss_type == AVAILABLE_TSS_386 || tss_type == BUSY_TSS_386) {
        if (cr3 != m_cr[3]) {
            cpu_prot_set_cr(3, cr3);
        }
    }
    m_phys_eip += (eip) - (m_phys_eip + m_eip_phys_bias);
    ;
    int eflags_mask = (tss_type == AVAILABLE_TSS_386 || tss_type == BUSY_TSS_386) ? -1 : 0xFFFF;
    cpu_set_eflags((eflags & eflags_mask) | (m_eflags & ~eflags_mask));
    for (int i = 0; i < 8; i++)
        m_reg32[i] = reg32[i];
    if (eflags & 0x20000) {
        for (int i = 0; i < 6; i++)
            cpu_seg_load_virtual(i, seg[i]);
        m_cpl = 3;
    } else {
        for (int i = 0; i < 6; i++)
            m_seg[i] = seg[i];
        m_cpl = seg[1] & 3;
    }
    for (int i = 0; i < 8; i++)
        m_reg32[i] = reg32[i];
    if (((ldt & 4) != 0)) {
            cpu_exception(10, (offset) | 0x10000);
            return 1;
        }
    int ldt_offset = ldt & 0xFFFC;
    if (ldt_offset) {
        struct seg_desc ldt_info;
        if (cpu_seg_load_descriptor2(7, ldt, &ldt_info, 10, ldt_offset))
            return 1;
        int ldt_access = ((&ldt_info)->raw[1] >> 8 & 0xFFFF);
        if (((ldt_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)) != 2){
                cpu_exception(10, (ldt_offset) | 0x10000);
                return 1;
            }
        if ((ldt_access & 0x80) == 0) {
                cpu_exception(10, (ldt_offset) | 0x10000);
                return 1;
            }
        m_seg[8]        = ldt;
        m_seg_base[8]   = cpu_seg_get_base(&ldt_info);
        m_seg_limit[8]  = cpu_seg_get_limit(&ldt_info);
        m_seg_access[8] = ldt_access;
    }
    for (int i = 0; i < 6; i++) {
        int             sel = seg[i], sel_offs = seg[i] & 0xFFFC, seg_access;
        struct seg_desc seg_info;
        switch (i) {
            case 1:
            case 2:
                if (!sel_offs) {
                        cpu_exception(10, (0) | 0x10000);
                        return 1;
                    }
                if (cpu_seg_load_descriptor(sel, &seg_info, 10, sel_offs))
                    return 1;
                seg_access = (seg_info.raw[1] >> 8 & 0xFFFF);
                if (!(seg_access & 0x80)) {
                        cpu_exception(10, (sel_offs) | 0x10000);
                        return 1;
                    }
                switch (((seg_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
                    case 0x12:
                    case 0x13:
                    case 0x16:
                    case 0x17:
                        if (i != 2)
                            goto error;
                        if (m_cpl != (sel & 3) && m_cpl != (seg_access >> 5 & 3)) {
                                cpu_exception(10, (sel_offs) | 0x10000);
                                return 1;
                            }
                        break;
                    case 0x18 ... 0x1B:
                        if (i != 1)
                            goto error;
                        if ((seg_access >> 5 & 3) != (sel & 3)) {
                                cpu_exception(10, (sel_offs) | 0x10000);
                                return 1;
                            }
                        break;
                    case 0x1C ... 0x1F:
                        if (i != 1)
                            goto error;
                        if ((seg_access >> 5 & 3) > (sel & 3)) {
                                cpu_exception(10, (sel_offs) | 0x10000);
                                return 1;
                            }
                        break;
                    default:
                    error: {
                            cpu_exception(10, (sel_offs) | 0x10000);
                            return 1;
                        }
                }
                if (cpu_seg_load_protected(i, sel, &seg_info))
                    return 1;
                break;
            default:
                if (!sel_offs) {
                    m_seg_base[i]   = 0;
                    m_seg_limit[i]  = 0;
                    m_seg_access[i] = 0;
                    continue;
                }
                if (cpu_seg_load_descriptor(sel, &seg_info, 10, sel_offs))
                    return 1;
                seg_access = (seg_info.raw[1] >> 8 & 0xFFFF);
                if (!(seg_access & 0x80)) {
                        cpu_exception(10, (sel_offs) | 0x10000);
                        return 1;
                    }
                switch (((seg_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
                    case 0x10 ... 0x17:
                    case 0x1A:
                    case 0x1B: {
                        int dpl = (seg_access >> 5 & 3);
                        if (dpl < (sel & 3) || dpl < m_cpl) {
                                cpu_exception(10, (sel_offs) | 0x10000);
                                return 1;
                            }
                        break;
                    }
                    case 0x1E:
                    case 0x1F:
                        break;
                    default: {
                            cpu_exception(10, (sel_offs) | 0x10000);
                            return 1;
                        }
                }
                if (cpu_seg_load_protected(i, sel, &seg_info))
                    return 1;
                break;
        }
    }
    return 0;
}

int CpuInternal::cpu_interrupt(int vector, int error_code, int type, int eip_to_push)
{
    DWORD STACK_esp, STACK_ss_base, STACK_esp_mask, STACK_mask, STACK_original_esp;

    if (m_cr[0] & 1) {
        if (m_eflags & 0x20000 && type == INTERRUPT_TYPE_SOFTWARE) {
            if (m_cr[4] & (1 << 0)) {
                WORD redirection_map_index, new_eip, new_cs;
                BYTE  redirection_map_entry;
                DWORD idt_entry;
                if (m_seg_limit[6] < 0x67) {
                        cpu_exception(13, (0) | 0x10000);
                        return 1;
                    }
                {
                    DWORD addr_ = m_seg_base[6] + 0x66, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> 0, 0))
                            redirection_map_index = m_read_result;
                        else
                            return 1;
                    } else {
                        redirection_map_entry = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                {
                    DWORD addr_ = redirection_map_index - 1 - ((~vector & 0xFF) >> 3) + m_seg_base[6], shift_ = 0,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> 0, 0))
                            redirection_map_entry = m_read_result;
                        else
                            return 1;
                    } else {
                        redirection_map_entry = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                if (!(redirection_map_entry & (1 << (vector & 7)))) {
                    DWORD flags_image = cpu_get_eflags();
                    if ((m_eflags >> 12 & 3) < 3) {
                        flags_image &= ~0x200;
                        flags_image |= (flags_image & 0x80000) ? 0x200 : 0;
                        flags_image |= 0x3000;
                    }
                    {
                        DWORD addr_ = vector << 2, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> 0, 0))
                                idt_entry = m_read_result;
                            else
                                return 1;
                        } else {
                            idt_entry = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    new_eip            = idt_entry & 0xFFFF;
                    new_cs             = idt_entry >> 16 & 0xFFFF;
                    STACK_esp          = m_reg32[4] & m_esp_mask;
                    STACK_original_esp = m_reg32[4];
                    STACK_ss_base      = m_seg_base[2];
                    STACK_esp_mask     = m_esp_mask;
                    STACK_mask         = 6;
                    ;
                    {
                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                       {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = flags_image,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                    }
                    {
                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                    }
                    {
                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = eip_to_push,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                            }
                        }
                    }
                    int bit_to_mask_out = (m_eflags >> 12 & 3) == 3 ? 0x200 : 0x80000;
                    m_eflags &= ~(bit_to_mask_out | 0x100);
                    cpu_load_csip_virtual(new_cs, new_eip);
                    m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                    return 0;
                }
            } else if ((m_eflags >> 12 & 3) < 3) {
                    cpu_exception(13, (0) | 0x10000);
                    return 1;
            }
        }
        int             offset = vector << 3, is_hardware = type == INTERRUPT_TYPE_HARDWARE;
        struct seg_desc idt_entry;
        if (cpu_seg_load_descriptor2(9, offset, &idt_entry, 13, (offset) | (1 << 1) | (is_hardware)))
            return 1;
        int idt_access     = ((&idt_entry)->raw[1] >> 8 & 0xFFFF);
        int idt_entry_type = ((idt_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
        switch (idt_entry_type) {
            case TASK_GATE: {
                if (!(idt_access & 0x80)) {
                        cpu_exception(11, ((offset) | (1 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }

                WORD tss_selector = cpu_seg_gate_target_segment(&idt_entry), tss_offset = tss_selector & 0xFFFC;
                struct seg_desc tss_entry;
                int             tss_access;
                if (((tss_selector & 4) != 0)) {
                        cpu_exception(10, ((tss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                if (cpu_seg_load_descriptor(tss_selector, &tss_entry, 13, (tss_offset) | (0 << 1) | (is_hardware)))
                    return 1;

                tss_access = ((&tss_entry)->raw[1] >> 8 & 0xFFFF);
                if (((tss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)) & 0x10) {
                        cpu_exception(13, ((tss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                if (!(tss_access & 0x80))  {
                        cpu_exception(11, ((tss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                if (do_task_switch(tss_selector, &tss_entry, TASK_INT, eip_to_push))
                    return 1;
                if (error_code & 0x10000) {
                    error_code &= 0xFFFF;
                    STACK_esp          = m_reg32[4] & m_esp_mask;
                    STACK_original_esp = m_reg32[4];
                    STACK_ss_base      = m_seg_base[2];
                    STACK_esp_mask     = m_esp_mask;
                    STACK_mask         = 2;
                    ;
                    if (((tss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)) == AVAILABLE_TSS_286 ||
                        ((tss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01)) == BUSY_TSS_286)
                        {
                            STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = error_code,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 1)) {
                                    if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                        return 1;
                                } else {
                                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                }
                            }
                        }
                    else {
                            STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = error_code,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 1)) {
                                    if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                        return 1;
                                } else {
                                    m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                }
                            }
                        }
                }
                break;
            }
            case INTERRUPT_GATE_286:
            case INTERRUPT_GATE_386:
            case TRAP_GATE_286:
            case TRAP_GATE_386: {
                int dpl = (idt_access >> 5 & 3);
                if (type == INTERRUPT_TYPE_SOFTWARE && dpl < m_cpl) {
                        cpu_exception(13, ((offset) | (1 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                if (!(idt_access & 0x80)) {
                        cpu_exception(11, ((offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                DWORD        cs = cpu_seg_gate_target_segment(&idt_entry), cs_offset = cs & 0xFFFC;
                DWORD        eip = cpu_seg_gate_target_offset(&idt_entry);
                struct seg_desc cs_info;
                int             cs_access;
                if (!cs_offset) {
                        cpu_exception(13, ((0) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                if (cpu_seg_load_descriptor(cs, &cs_info, 13, (cs_offset) | (0 << 1) | (is_hardware)))
                    return 1;
                cs_access = ((&cs_info)->raw[1] >> 8 & 0xFFFF);
                int type  = ((cs_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                if (!(type >= 0x18 && type <= 0x1F)) {
                        cpu_exception(13, ((cs_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                dpl = (cs_access >> 5 & 3);
                if (dpl > m_cpl) {
                        cpu_exception(13, ((cs_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                if (!(idt_access & 0x80))  {
                        cpu_exception(11, ((cs_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                        return 1;
                    }
                int             esp, ss, ss_offset;
                struct seg_desc ss_info;
                DWORD        old_esp = m_reg32[4], old_ss = m_seg[2], esp_mask, ss_base;
                int             changed_privilege_level = 0;
                switch (type) {
                    case 0x18 ... 0x1B: {
                        if (dpl == m_cpl)
                            goto conforming;
                        if (!(dpl < m_cpl))
                            goto error;
                        if (dpl != 0 && m_eflags & 0x20000) {
                                cpu_exception(13, ((cs_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                                return 1;
                            }

                        if (get_tss_esp(dpl, &esp))
                            return 1;
                        if (get_tss_ss(dpl, &ss))
                            return 1;
                        ss_offset               = ss & 0xFFFC;
                        changed_privilege_level = 1;
                        if (!ss_offset) {
                                cpu_exception(10, ((ss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                                return 1;
                            }
                        if (cpu_seg_load_descriptor(ss, &ss_info, 10, (ss_offset) | (0 << 1) | (is_hardware)))
                            return 1;
                        int ss_access = ((&ss_info)->raw[1] >> 8 & 0xFFFF);
                        if ((ss & 3) != (unsigned int)dpl || (ss_access >> 5 & 3) != (unsigned int)dpl) {
                                cpu_exception(10, ((ss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                                return 1;
                            }

                        int type = ((ss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                        if (!(type == 0x12 || type == 0x13 || type == 0x16 || type == 0x17)) {
                                cpu_exception(10, ((ss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                                return 1;
                            }

                        if (!(ss_access & 0x80)) {
                                cpu_exception(12, ((ss_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                                return 1;
                            }

                        esp_mask           = ss_access & 0x4000 ? -1 : 0xFFFF;
                        ss_base            = cpu_seg_get_base(&ss_info);
                        esp                = (esp & esp_mask) | (m_reg32[4] & ~esp_mask);
                        STACK_esp          = esp & esp_mask;
                        STACK_original_esp = esp;
                        STACK_ss_base      = ss_base;
                        STACK_esp_mask     = esp_mask;
                        STACK_mask         = cpl_to_TLB_write[dpl];
                        ;
                        if (idt_entry_type & 8) {
                            if (m_eflags & 0x20000) {
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[5], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::DWord, addr, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[4], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::DWord, addr, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[3], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::DWord, addr, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[0], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::DWord, addr, data_);
                                        }
                                    }
                                }
                                m_seg[5]        = 0;
                                m_seg_limit[5]  = 0;
                                m_seg_base[5]   = 0;
                                m_seg_access[5] = 0;
                                m_seg[4]        = 0;
                                m_seg_limit[4]  = 0;
                                m_seg_base[4]   = 0;
                                m_seg_access[4] = 0;
                                m_seg[3]        = 0;
                                m_seg_limit[3]  = 0;
                                m_seg_base[3]   = 0;
                                m_seg_access[3] = 0;
                                m_seg[0]        = 0;
                                m_seg_limit[0]  = 0;
                                m_seg_base[0]   = 0;
                                m_seg_access[0] = 0;
                            }
                            {
                                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = old_ss,
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 3)) {
                                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        auto addr = m_tlb[addr_ >> 12] + addr_;
                                        m_ram->Push(DataFormat::DWord, addr, data_);
                                    }
                                }
                            }
                            {
                                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = old_esp,
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 3)) {
                                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        auto addr = m_tlb[addr_ >> 12] + addr_;
                                        m_ram->Push(DataFormat::DWord, addr, data_);
                                    }
                                }
                            }
                        } else {
                            if (m_eflags & 0x20000) {
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[5], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::Word, addr, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[4], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::Word, addr, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[3], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::Word, addr, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[0], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            auto addr = m_tlb[addr_ >> 12] + addr_;
                                            m_ram->Push(DataFormat::Word, addr, data_);
                                        }
                                    }
                                }
                                m_seg[5]        = 0;
                                m_seg_limit[5]  = 0;
                                m_seg_base[5]   = 0;
                                m_seg_access[5] = 0;
                                m_seg[4]        = 0;
                                m_seg_limit[4]  = 0;
                                m_seg_base[4]   = 0;
                                m_seg_access[4] = 0;
                                m_seg[3]        = 0;
                                m_seg_limit[3]  = 0;
                                m_seg_base[3]   = 0;
                                m_seg_access[3] = 0;
                                m_seg[0]        = 0;
                                m_seg_limit[0]  = 0;
                                m_seg_base[0]   = 0;
                                m_seg_access[0] = 0;
                            }
                            {
                                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = old_ss,
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 1)) {
                                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        auto addr = m_tlb[addr_ >> 12] + addr_;
                                        m_ram->Push(DataFormat::Word, addr, data_);
                                    }
                                }
                            }
                            {
                                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = old_esp,
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 1)) {
                                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        auto addr = m_tlb[addr_ >> 12] + addr_;
                                        m_ram->Push(DataFormat::Word, addr, data_);
                                    }
                                }
                            }
                        }
                    } break;
                    case 0x1C ... 0x1F:
                    conforming:
                        if (dpl != m_cpl && m_eflags & 0x20000)
                            goto error;
                        ss                 = m_seg[2];
                        ss_base            = m_seg_base[2];
                        esp                = m_reg32[4];
                        esp_mask           = m_esp_mask;
                        STACK_esp          = esp & esp_mask;
                        STACK_original_esp = esp;
                        STACK_ss_base      = ss_base;
                        STACK_esp_mask     = esp_mask;
                        STACK_mask         = cpl_to_TLB_write[dpl];
                        ;
                        break;
                    default:
                    error: {
                            cpu_exception(13, ((cs_offset) | (0 << 1) | (is_hardware)) | 0x10000);
                            return 1;
                        }
                }
                if (idt_entry_type & 8) {
                    {
                        STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = cpu_get_eflags(),
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                auto addr = m_tlb[addr_ >> 12] + addr_;
                                m_ram->Push(DataFormat::DWord, addr, data_);
                            }
                        }
                    }
                    {
                        STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                auto addr = m_tlb[addr_ >> 12] + addr_;
                                m_ram->Push(DataFormat::DWord, addr, data_);
                            }
                        }
                    }

                    {
                        STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = eip_to_push,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                auto addr = m_tlb[addr_ >> 12] + addr_;
                                m_ram->Push(DataFormat::DWord, addr, data_);
                            }
                        }
                    }

                    if (error_code & 0x10000) {
                            STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         data_ = error_code & 0xFFFF, tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                        return 1;
                                } else {
                                    auto addr = m_tlb[addr_ >> 12] + addr_;
                                    m_ram->Push(DataFormat::DWord, addr, data_);
                                }
                            }
                        }
                } else {
                    {
                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = cpu_get_eflags(),
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                auto addr = m_tlb[addr_ >> 12] + addr_;
                                m_ram->Push(DataFormat::Word, addr, data_);
                            }
                        }
                    }
                    {
                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                auto addr = m_tlb[addr_ >> 12] + addr_;
                                m_ram->Push(DataFormat::Word, addr, data_);
                            }
                        }
                    }
                    {
                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = eip_to_push,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                    return 1;
                            } else {
                                auto addr = m_tlb[addr_ >> 12] + addr_;
                                m_ram->Push(DataFormat::Word, addr, data_);
                            }
                        }
                    }

                    if (error_code & 0x10000) {
                            STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         data_ = error_code & 0xFFFF, tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 1)) {
                                    if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                        return 1;
                                } else {
                                    auto addr = m_tlb[addr_ >> 12] + addr_;
                                    m_ram->Push(DataFormat::Word, addr, data_);
                                }
                            }
                        }
                }
                m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                if (changed_privilege_level) {
                    if (cpu_seg_load_protected(2, (ss & ~3) | dpl, &ss_info))
                        return 1;
                    if (cpu_load_csip_protected((cs & ~3) | dpl, &cs_info, eip))
                        return 1;
                } else if (cpu_load_csip_protected((cs & ~3) | m_cpl, &cs_info, eip))
                    return 1;
                m_eflags &= ~(0x100 | 0x20000 | 0x10000 | 0x4000);
                cpu_prot_update_cpl();
                if (!(idt_entry_type & 1))
                    m_eflags &= ~0x200;
                break;
            }
            default: {
                    cpu_exception(13, ((offset) | (1 << 1) | (is_hardware)) | 0x10000);
                    return 1;
                }
        }
        return 0;
    } else {
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = 2;
        ;
        {
            STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
           {
                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = cpu_get_eflags() & 0xFFFF,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                        return 1;
                } else {
                    auto addr = m_tlb[addr_ >> 12] + addr_;
                    m_ram->Push(DataFormat::Word, addr, data_);
                }
            }
        }
        {
            STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
            {
                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                        return 1;
                } else {
                    auto addr = m_tlb[addr_ >> 12] + addr_;
                    m_ram->Push(DataFormat::Word, addr, data_);
                }
            }
        }
        {
            STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
            {
                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = eip_to_push,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                        return 1;
                } else {
                    auto addr = m_tlb[addr_ >> 12] + addr_;
                    m_ram->Push(DataFormat::Word, addr, data_);
                }
            }
        }

        m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
        vector <<= 2;

        WORD cs = m_ram->Fetch(DataFormat::Word, vector + 2);
        WORD eip = m_ram->Fetch(DataFormat::Word, vector);

        cpu_load_csip_real(cs, eip);
        m_eflags &= ~(0x200 | 0x100 | 0x40000);
        return 0;
    }
}
void CpuInternal::cpu_exception(int vec, int code)
{
    while (1) {
        if (m_current_exception >= 0) {
            if (m_current_exception == 8) {
                util_abort();
            }
            m_current_exception = 8;
            vec                 = 8;
            code                = 0 | 0x10000;
        }

        m_current_exception = vec;
        if (cpu_interrupt(vec, code, INTERRUPT_TYPE_EXCEPTION, (m_phys_eip + m_eip_phys_bias)))
            m_current_exception = vec;
        else
            break;
    }
    m_current_exception = -1;
}
int CpuInternal::jmpf(DWORD eip, DWORD cs, DWORD eip_after)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000) {
        cpu_load_csip_real(cs, eip);
    } else {
        DWORD        offset = cs & ~3, access;
        int             dpl, type, rpl = cs & 3;
        struct seg_desc info;
        if (offset == 0)
            do {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            } while (0);
        if (cpu_seg_load_descriptor(cs, &info, 13, offset))
            return 1;
        access = ((&info)->raw[1] >> 8 & 0xFFFF);
        if ((access & 0x80) == 0)
            do {
                cpu_exception(11, (offset) | 0x10000);
                return 1;
            } while (0);
        dpl  = (access >> 5 & 3);
        type = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
        switch (type) {
            case 0x18 ... 0x1B:
                if (rpl > m_cpl || dpl != m_cpl)
                    do {
                        cpu_exception(13, (offset) | 0x10000);
                        return 1;
                    } while (0);
                if (cpu_load_csip_protected(offset | m_cpl, &info, eip))
                    return 1;
                break;
            case 0x1C ... 0x1F:
                if (dpl > m_cpl)
                    do {
                        cpu_exception(13, (offset) | 0x10000);
                        return 1;
                    } while (0);
                if (cpu_load_csip_protected(offset | m_cpl, &info, eip))
                    return 1;
                break;
            case CALL_GATE_286:
            case CALL_GATE_386: {
                DWORD        gate_cs, gate_eip, gate_cs_offset;
                struct seg_desc gate_info;
                if (dpl < m_cpl || dpl < rpl)
                    do {
                        cpu_exception(13, (offset) | 0x10000);
                        return 1;
                    } while (0);
                gate_cs        = cpu_seg_gate_target_segment(&info);
                gate_eip       = cpu_seg_gate_target_offset(&info);
                gate_cs_offset = gate_cs & ~3;
                if (cpu_seg_load_descriptor(gate_cs, &gate_info, 13, gate_cs_offset))
                    return 1;
                access = ((&gate_info)->raw[1] >> 8 & 0xFFFF);
                dpl    = (access >> 5 & 3);
                switch (((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
                    case 0x1C ... 0x1F:
                        if (dpl > m_cpl)
                            do {
                                cpu_exception(13, (gate_cs_offset) | 0x10000);
                                return 1;
                            } while (0);
                        break;
                    case 0x18 ... 0x1B:
                        if (dpl != m_cpl)
                            do {
                                cpu_exception(13, (gate_cs_offset) | 0x10000);
                                return 1;
                            } while (0);
                        break;
                    default:
                        do {
                            cpu_exception(13, (gate_cs_offset) | 0x10000);
                            return 1;
                        } while (0);
                }
                if ((access & 0x80) == 0)
                    do {
                        cpu_exception(11, (gate_cs_offset) | 0x10000);
                        return 1;
                    } while (0);
                gate_eip &= type == CALL_GATE_386 ? -1 : 0xFFFF;
                if (cpu_load_csip_protected(gate_cs_offset | m_cpl, &gate_info, gate_eip))
                    return 1;
                break;
            }
            case AVAILABLE_TSS_286:
            case AVAILABLE_TSS_386:
                if (dpl < m_cpl || dpl < rpl)
                    do {
                        cpu_exception(13, (offset) | 0x10000);
                        return 1;
                    } while (0);
                if (!(access & 0x80))
                    do {
                        cpu_exception(11, (offset) | 0x10000);
                        return 1;
                    } while (0);
                if (do_task_switch(cs, &info, TASK_JMP, eip_after))
                    return 1;
                break;
            case TASK_GATE: {
                if (dpl < m_cpl || dpl < rpl)
                    do {
                        cpu_exception(13, (offset) | 0x10000);
                        return 1;
                    } while (0);
                if (load_tss_from_task_gate(&cs, &info))
                    return 1;
                if (do_task_switch(cs, &info, TASK_JMP, eip_after))
                    return 1;
                break;
            }
        }
    }
    return 0;
}
DWORD CpuInternal::call_gate_read_param32(DWORD addr, DWORD *dest, int mask)
{
    if ((addr + 3) > m_seg_limit[2]){
            cpu_exception(12, (0) | 0x10000);
            return 1;
        }

    {
        DWORD addr_ = addr + m_seg_base[2], shift_ = mask, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> mask, mask))
                *dest = m_read_result;
            else
                return 1;
        } else {
            *dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }
    return 0;
}
WORD CpuInternal::call_gate_read_param16(DWORD addr, DWORD *dest, int mask)
{
    if ((addr + 1) > m_seg_limit[2]) {
            cpu_exception(12, (0) | 0x10000);
            return 1;
        }
    {
        DWORD addr_ = addr + m_seg_base[2], shift_ = mask, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> mask, mask))
                *dest = m_read_result;
            else
                return 1;
        } else {
            *dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }

    return 0;
}
int CpuInternal::callf(DWORD eip, DWORD cs, DWORD oldeip, int is32)
{
    DWORD STACK_esp, STACK_ss_base, STACK_esp_mask, STACK_mask, STACK_original_esp;
    ;
    if ((m_cr[0] & 1) && !(m_eflags & 0x20000)) {
        cs &= 0xFFFF;
        int cs_offset = cs & 0xFFFC, cs_access, cs_type, cs_dpl, cs_rpl;
        if (!cs_offset) {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }

        struct seg_desc cs_info;
        if (cpu_seg_load_descriptor(cs, &cs_info, 13, cs_offset))
            return 1;
        cs_access = ((&cs_info)->raw[1] >> 8 & 0xFFFF);

        if ((cs_access & 0x80) == 0) {
                cpu_exception(11, (cs_offset) | 0x10000);
                return 1;
            }
        cs_type = ((cs_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
        cs_dpl  = (cs_access >> 5 & 3);
        cs_rpl  = (cs & 3);
        switch (cs_type) {
            case 0x1C ... 0x1F:
                if (cs_dpl > m_cpl) {
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }
                break;
            case 0x18 ... 0x1B:
                if (cs_rpl > m_cpl || cs_dpl != m_cpl) {
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }
                break;
            case CALL_GATE_286:
            case CALL_GATE_386: {
                if (cs_dpl < m_cpl || cs_dpl < cs_rpl) {
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }

                DWORD gate_cs = cpu_seg_gate_target_segment(&cs_info), gate_cs_offset = gate_cs & 0xFFFC,
                         gate_eip = cpu_seg_gate_target_offset(&cs_info);
                int             gate_dpl, dpldiff, gate_type, gate_access;
                struct seg_desc gate_info;
                if (!gate_cs_offset){
                        cpu_exception(13, (0) | 0x10000);
                        return 1;
                    }

                if (cpu_seg_load_descriptor(gate_cs, &gate_info, 13, gate_cs_offset))
                    return 1;
                gate_access = ((&gate_info)->raw[1] >> 8 & 0xFFFF);
                gate_dpl    = (gate_access >> 5 & 3);
                gate_type   = ((gate_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                dpldiff     = gate_dpl - m_cpl;
                switch (gate_type) {
                    case 0x18 ... 0x1B:
                        if (dpldiff < 0) {
                            int             ss, esp, ss_offset, ss_access, ss_type, ss_base, ss_mask;
                            struct seg_desc ss_info;
                            if (get_tss_ss(gate_dpl, &ss))
                                return 1;
                            if (get_tss_esp(gate_dpl, &esp))
                                return 1;
                            ss_offset = ss & 0xFFFC;
                            if (!ss_offset) {
                                    cpu_exception(10, (0) | 0x10000);
                                    return 1;
                                }

                            if (cpu_seg_load_descriptor(ss, &ss_info, 10, ss_offset))
                                return 1;
                            ss_access = (ss_info.raw[1] >> 8 & 0xFFFF);
                            ss_type   = ((ss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                            if ((unsigned int)gate_dpl != (ss_access >> 5 & 3)) {
                                    cpu_exception(10, (ss_offset) | 0x10000);
                                    return 1;
                                }
                            if (!(ss_type == 0x12 || ss_type == 0x13 || ss_type == 0x16 || ss_type == 0x17)){
                                    cpu_exception(10, (ss_offset) | 0x10000);
                                    return 1;
                                }
                            if ((ss_access & 0x80) == 0){
                                    cpu_exception(10, (ss_offset) | 0x10000);
                                    return 1;
                                }
                            int       parameter_count = cpu_seg_gate_parameter_count(&cs_info);

                            std::vector<DWORD> paramsTmp;
                            paramsTmp.reserve(parameter_count);

                            DWORD *params          = paramsTmp.data();
                            ss_base                   = cpu_seg_get_base(&ss_info);
                            ss_mask                   = ss_access & 0x4000 ? -1 : 0xFFFF;
                            DWORD old_esp          = m_reg32[4] & m_esp_mask;
                            for (int i = parameter_count - 1, j = 0; i >= 0; i--, j++) {
                                if (cs_type == CALL_GATE_386) {
                                    if (call_gate_read_param32(((old_esp + (i << 2)) & m_esp_mask), &params[j],
                                                               gate_dpl))
                                        return 1;
                                } else {
                                    if (call_gate_read_param16(((old_esp + (i << 1)) & m_esp_mask), &params[j],
                                                               gate_dpl))
                                        return 1;
                                }
                            }
                            esp                = (esp & ss_mask) | (m_reg32[4] & ~ss_mask);
                            STACK_esp          = esp & ss_mask;
                            STACK_original_esp = esp;
                            STACK_ss_base      = ss_base;
                            STACK_esp_mask     = ss_mask;
                            STACK_mask         = cpl_to_TLB_write[gate_dpl];
                            ;
                            if (cs_type == CALL_GATE_386) {
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                   {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[2], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = old_esp, tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }

                                for (int i = 0; i < parameter_count; i++) {
                                    {
                                        STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                        {
                                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                     data_ = params[i], tag = m_tlb_tags[addr_ >> 12];
                                            if (((addr_ | tag >> shift_) & 3)) {
                                                if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                    return 1;
                                            } else {
                                                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                            }
                                        }
                                    }
                                }
                               {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[1], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                   {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                                                 tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 3)) {
                                            if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    };
                                }
                            } else {
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[2], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = old_esp, tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }
                                for (int i = 0; i < parameter_count; i++) {
                                    {
                                        STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                        {
                                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                     data_ = params[i], tag = m_tlb_tags[addr_ >> 12];
                                            if (((addr_ | tag >> shift_) & 1)) {
                                                if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                    return 1;
                                            } else {
                                                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                            }
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                                 data_ = m_seg[1], tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }
                                {
                                    STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                    {
                                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                                                 tag = m_tlb_tags[addr_ >> 12];
                                        if (((addr_ | tag >> shift_) & 1)) {
                                            if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                                return 1;
                                        } else {
                                            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                        }
                                    }
                                }
                            }

                            if (cpu_seg_load_protected(2, (ss & ~3) | gate_dpl, &ss_info))
                                return 1;
                            m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                            if (cpu_load_csip_protected((gate_cs & ~3) | gate_dpl, &gate_info, gate_eip))
                                return 1;
                            return 0;

                        } else if (dpldiff > 0){
                                cpu_exception(13, (gate_cs_offset) | 0x10000);
                                return 1;
                        } else
                            goto __workaround_gcc;
                    case 0x1C ... 0x1F:
                    __workaround_gcc:
                        STACK_esp          = m_reg32[4] & m_esp_mask;
                        STACK_original_esp = m_reg32[4];
                        STACK_ss_base      = m_seg_base[2];
                        STACK_esp_mask     = m_esp_mask;
                        STACK_mask         = m_tlb_shift_write;
                        ;
                        if (cs_type == CALL_GATE_386) {
                            {
                                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 3)) {
                                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                    }
                                }
                            }
                            {
                                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                               {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 3)) {
                                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                                    }
                                }
                            }
                        } else {
                            {
                                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 1)) {
                                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                    }
                                }
                            }
                            {
                                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                                {
                                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                                             tag = m_tlb_tags[addr_ >> 12];
                                    if (((addr_ | tag >> shift_) & 1)) {
                                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                                            return 1;
                                    } else {
                                        m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                                    }
                                }
                            }
                        }
                        if (cpu_load_csip_protected((gate_cs & ~3) | m_cpl, &gate_info, gate_eip))
                            return 1;
                        m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                        return 0;
                    default: {
                            cpu_exception(13, (gate_cs_offset) | 0x10000);
                            return 1;
                        }
                }
                util_abort();
                break;
            }
            case AVAILABLE_TSS_286:
            case AVAILABLE_TSS_386:
                if (cs_dpl < m_cpl || cs_dpl < cs_rpl){
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }
                do_task_switch(cs, &cs_info, TASK_CALL, eip);
                return 0;
            case TASK_GATE:
                if (cs_dpl < m_cpl || cs_dpl < cs_rpl){
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }
                if (load_tss_from_task_gate(&cs, &cs_info))
                    return 1;
                if (do_task_switch(cs, &cs_info, TASK_JMP, eip))
                    return 1;
                return 0;
            default: {
                    cpu_exception(13, (cs_offset) | 0x10000);
                    return 1;
                }
        }
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = m_tlb_shift_write;
        ;
        if (is32) {
            {
                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
            {
                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
        } else {
            {
                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
           {
                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
        }
        if (cpu_load_csip_protected((cs & ~3) | m_cpl, &cs_info, eip))
            return 1;
        m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
        return 0;
    } else {
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = m_tlb_shift_write;
        ;
        if (is32) {
             {
                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
            {
                STACK_esp = (STACK_esp - 4) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (cpu_access_write32(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
        } else {
            {
                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = m_seg[1],
                            tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
            {
                STACK_esp = (STACK_esp - 2) & STACK_esp_mask;
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, data_ = oldeip,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (cpu_access_write16(addr_, data_, tag >> STACK_mask, STACK_mask))
                            return 1;
                    } else {
                        m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
                    }
                }
            }
        }
        m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
        if (m_cr[0] & 1)
            cpu_load_csip_virtual(cs, eip);
        else
            cpu_load_csip_real(cs, eip);
        return 0;
    }
}
void CpuInternal::iret_handle_seg(int x)
{
    WORD access  = m_seg_access[x];
    int      invalid = 0, type = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
    if ((m_seg[x] & 0xFFFC) == 0)
        invalid = 1;
    else if (m_cpl > (access >> 5 & 3)) {
        switch (type) {
            case 0x1C ... 0x1F:
            case 0x10 ... 0x17:
                invalid = 1;
                break;
        }
    }
    if (invalid) {
        m_seg[x]        = 0;
        m_seg_access[x] = 0;
        m_seg_base[x]   = 0;
        m_seg_limit[x]  = 0;
        m_seg_valid[x]  = 0;
    }
}
int CpuInternal::iret(DWORD tss_eip, int is32)
{
    DWORD STACK_esp, STACK_ss_base, STACK_esp_mask, STACK_mask, STACK_original_esp;
    ;
    DWORD eip = 0, cs = 0, eflags = 0;
    if (m_cr[0] & 1) {
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = m_tlb_shift_write;
        ;
        if (m_eflags & 0x20000) {
            if ((m_eflags >> 12 & 3) == 3) {
                int eflags_mask;
                if (is32) {
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                    eip = m_read_result;
                                else
                                    return 1;
                            } else {
                                eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                    cs = m_read_result;
                                else
                                    return 1;
                            } else {
                                cs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                    eflags = m_read_result;
                                else
                                    return 1;
                            } else {
                                eflags = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                    }
                    eflags_mask = 0x20000 | 0x3000 | 0x100000 | 0x80000;
                } else {
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    eip = m_read_result;
                                else
                                    return 1;
                            } else {
                                eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    {
                       {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    cs = m_read_result;
                                else
                                    return 1;
                            } else {
                                cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    {
                       {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    eflags = m_read_result;
                                else
                                    return 1;
                            } else {
                                eflags = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    eflags_mask = 0x3000 | 0xFFFF0000;
                }
                m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                cpu_load_csip_virtual(cs, eip);
                cpu_set_eflags((eflags & ~eflags_mask) | (m_eflags & eflags_mask));
            } else {
                if (m_cr[4] & (1 << 0)) {
                    if (is32)
                        util_abort();
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    eip = m_read_result;
                                else
                                    return 1;
                            } else {
                                eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    cs = m_read_result;
                                else
                                    return 1;
                            } else {
                                cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    eflags = m_read_result;
                                else
                                    return 1;
                            } else {
                                eflags = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    if ((m_eflags & 0x100000 && eflags & (1 << 9)) || eflags & (1 << 8)) {
                            cpu_exception(13, (0) | 0x10000);
                            return 1;
                        }

                    m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                    cpu_load_csip_virtual(cs, eip);
                    const DWORD mask = 0xFFFF ^ (0x3000 | 0x200);
                    if (eflags & 0x200)
                        m_eflags |= 0x80000;
                    else
                        m_eflags &= ~0x80000;
                    cpu_set_eflags((eflags & mask) | (m_eflags & ~mask));
                } else {
                        cpu_exception(13, (0) | 0x10000);
                        return 1;
                    }
            }
        } else {
            if (m_eflags & 0x4000) {
                WORD        tss_back_link, tss_offset;
                struct seg_desc tss_info;
                {
                    DWORD addr_ = m_seg_base[6], shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> 0, 0))
                            tss_back_link = m_read_result;
                        else
                            return 1;
                    } else {
                        tss_back_link = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                tss_offset = tss_back_link & 0xFFFC;
                if (((tss_back_link & 4) != 0)) {
                        cpu_exception(10, (tss_back_link) | 0x10000);
                        return 1;
                    }

                if (cpu_seg_load_descriptor2(7, tss_back_link, &tss_info, 10, tss_offset))
                    return 1;
                int access = ((&tss_info)->raw[1] >> 8 & 0xFFFF),
                    type   = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                if (type == BUSY_TSS_286 || type == BUSY_TSS_386) {
                        cpu_exception(10, (tss_offset) | 0x10000);
                        return 1;
                    }

                return do_task_switch(tss_offset, &tss_info, TASK_IRET, tss_eip);
            } else {
                int             old_cpl = m_cpl;
                DWORD        cs_offset;
                struct seg_desc cs_info;
                DWORD        eflags_mask = is32 ? -1 : 0xFFFF;
                STACK_esp                   = m_reg32[4] & m_esp_mask;
                STACK_original_esp          = m_reg32[4];
                STACK_ss_base               = m_seg_base[2];
                STACK_esp_mask              = m_esp_mask;
                STACK_mask                  = m_tlb_shift_write;
                ;
                if (is32) {
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                    eip = m_read_result;
                                else
                                    return 1;
                            } else {
                                eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                    cs = m_read_result;
                                else
                                    return 1;
                            } else {
                                cs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                    eflags = m_read_result;
                                else
                                    return 1;
                            } else {
                                eflags = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                    }
                    cs &= 0xFFFF;
                    if (eflags & 0x20000 && m_cpl == 0) {
                        DWORD esp, ss, es, ds, fs, gs;
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        esp = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    esp = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        ss = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    ss = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        {
                           {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        es = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    es = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            };
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        ds = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    ds = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        {
                           {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        fs = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    fs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        gs = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    gs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        cpu_seg_load_virtual(0, es);
                        cpu_seg_load_virtual(3, ds);
                        cpu_seg_load_virtual(4, fs);
                        cpu_seg_load_virtual(5, gs);
                        cpu_seg_load_virtual(2, ss);
                        cpu_load_csip_virtual(cs, eip & 0xFFFF);
                        m_reg32[4] = esp;
                        cpu_set_eflags((eflags & eflags_mask) | (m_eflags & ~eflags_mask));
                        m_cpl = 3;
                        cpu_prot_update_cpl();
                        return 0;
                    }
                } else {
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    eip = m_read_result;
                                else
                                    return 1;
                            } else {
                                eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    cs = m_read_result;
                                else
                                    return 1;
                            } else {
                                cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                    {
                        {
                            DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                    eflags = m_read_result;
                                else
                                    return 1;
                            } else {
                                eflags = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                            }
                        }
                        STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                    }
                }
                cs_offset = cs & 0xFFFC;
                if (!cs_offset){
                        cpu_exception(13, (0) | 0x10000);
                        return 1;
                    }

                if (cpu_seg_load_descriptor(cs, &cs_info, 13, cs_offset))
                    return 1;
                int access = ((&cs_info)->raw[1] >> 8 & 0xFFFF), dpl = (access >> 5 & 3), rpl = (cs & 3);
                if (rpl < m_cpl){
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }

                switch (((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
                    case 0x18 ... 0x1B:
                        if (dpl != rpl){
                                cpu_exception(13, (cs_offset) | 0x10000);
                                return 1;
                            }
                        break;
                    case 0x1C ... 0x1F:
                        if (dpl > rpl){
                                cpu_exception(13, (cs_offset) | 0x10000);
                                return 1;
                            }
                        break;
                    default:{
                            cpu_exception(13, (cs_offset) | 0x10000);
                            return 1;
                        }
                }
                if ((access & 0x80) == 0){
                        cpu_exception(11, (cs_offset) | 0x10000);
                        return 1;
                    }
                if (rpl != m_cpl) {
                    DWORD        esp = 0, ss = 0, ss_offset, esp_mask;
                    int             ss_access, ss_type, ss_dpl;
                    struct seg_desc ss_info;
                    if (is32) {
                      {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        esp = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    esp = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 3)) {
                                    if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                        ss = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    ss = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                        }
                        ss &= 0xFFFF;
                    } else {
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 1)) {
                                    if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                        esp = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    esp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                        }
                        {
                            {
                                DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask,
                                         tag = m_tlb_tags[addr_ >> 12];
                                if (((addr_ | tag >> shift_) & 1)) {
                                    if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                        ss = m_read_result;
                                    else
                                        return 1;
                                } else {
                                    ss = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                                }
                            }
                            STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                        }
                    }
                    ss_offset = ss & 0xFFFC;
                    if (!ss_offset) {
                            cpu_exception(13, (0) | 0x10000);
                            return 1;
                        }

                    if (cpu_seg_load_descriptor(ss, &ss_info, 13, ss_offset))
                        return 1;
                    if ((ss & 3) != (unsigned int)rpl) {
                            cpu_exception(13, (ss_offset) | 0x10000);
                            return 1;
                        }

                    ss_access = ((&ss_info)->raw[1] >> 8 & 0xFFFF);
                    ss_type   = ((ss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                    ss_dpl    = (ss_access >> 5 & 3);
                    esp_mask  = ss_access & 0x4000 ? -1 : 0xFFFF;
                    if (!(ss_type == 0x12 || ss_type == 0x13 || ss_type == 0x16 || ss_type == 0x17)){
                            cpu_exception(13, (ss_offset) | 0x10000);
                            return 1;
                        }

                    if (ss_dpl != rpl) {
                            cpu_exception(13, (ss_offset) | 0x10000);
                            return 1;
                        }

                    if ((ss_access & 0x80) == 0) {
                            cpu_exception(11, (cs_offset) | 0x10000);
                            return 1;
                        }

                    if (cpu_seg_load_protected(2, ss, &ss_info))
                        return 1;
                    if (cpu_load_csip_protected(cs, &cs_info, eip))
                        return 1;
                    m_reg32[4] = (esp & esp_mask) | (m_reg32[4] & ~esp_mask);
                    iret_handle_seg(0);
                    iret_handle_seg(4);
                    iret_handle_seg(5);
                    iret_handle_seg(3);
                } else {
                    if (cpu_load_csip_protected(cs, &cs_info, eip))
                        return 1;
                    m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
                }
                DWORD flag_mask =
                    0x01 | 0x04 | 0x10 | 0x40 | 0x80 | 0x100 | 0x400 | 0x800 | 0x4000 | 0x10000 | 0x40000 | 0x200000;
                if ((unsigned int)old_cpl <= (m_eflags >> 12 & 3))
                    flag_mask |= 0x200;
                if (old_cpl == 0)
                    flag_mask |= 0x3000 | 0x80000 | 0x100000;
                if (!is32)
                    flag_mask &= 0xFFFF;
                cpu_set_eflags((eflags & flag_mask) | (m_eflags & ~flag_mask));
            }
        }
        return 0;
    } else {
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = m_tlb_shift_write;
        ;
        if (is32) {
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            eip = m_read_result;
                        else
                            return 1;
                    } else {
                        eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            cs = m_read_result;
                        else
                            return 1;
                    } else {
                        cs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
            {
               {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            eflags = m_read_result;
                        else
                            return 1;
                    } else {
                        eflags = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
        } else {
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            eip = m_read_result;
                        else
                            return 1;
                    } else {
                        eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            cs = m_read_result;
                        else
                            return 1;
                    } else {
                        cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            eflags = m_read_result;
                        else
                            return 1;
                    } else {
                        eflags = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
        }
        m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
        cpu_load_csip_real(cs, eip);
        if (is32)
            cpu_set_eflags((eflags & 0x257FD5) | (m_eflags & 0x1A0000));
        else
            cpu_set_eflags(eflags | (m_eflags & ~0xFFFF));
        return 0;
    }
}
int CpuInternal::retf(int adjust, int is32)
{
    DWORD STACK_esp, STACK_ss_base, STACK_esp_mask, STACK_mask, STACK_original_esp;

    DWORD eip = 0, cs = 0;
    if ((m_cr[0] & 1) == 0 || (m_eflags & 0x20000)) {
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = m_tlb_shift_write;
        ;
        if (is32) {
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            eip = m_read_result;
                        else
                            return 1;
                    } else {
                        eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            cs = m_read_result;
                        else
                            return 1;
                    } else {
                        cs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
        } else {
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            eip = m_read_result;
                        else
                            return 1;
                    } else {
                        eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            cs = m_read_result;
                        else
                            return 1;
                    } else {
                        cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
        }
        if ((m_phys_eip + m_eip_phys_bias) >= m_seg_limit[1]) {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }

        STACK_esp  = (STACK_esp + adjust) & STACK_esp_mask;
        m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
        if (m_cr[0] & 1)
            cpu_load_csip_virtual(cs, eip);
        else
            cpu_load_csip_real(cs, eip);
        return 0;
    } else {
        STACK_esp          = m_reg32[4] & m_esp_mask;
        STACK_original_esp = m_reg32[4];
        STACK_ss_base      = m_seg_base[2];
        STACK_esp_mask     = m_esp_mask;
        STACK_mask         = m_tlb_shift_write;
        ;
        if (is32) {
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            eip = m_read_result;
                        else
                            return 1;
                    } else {
                        eip = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
            {
               {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                            cs = m_read_result;
                        else
                            return 1;
                    } else {
                        cs = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
            }
        } else {
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            eip = m_read_result;
                        else
                            return 1;
                    } else {
                        eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
            {
                {
                    DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                            cs = m_read_result;
                        else
                            return 1;
                    } else {
                        cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                    }
                }
                STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
            }
        }
        cs &= 0xFFFF;
        DWORD        cs_offset = cs & 0xFFFC;
        int             access, rpl, dpl;
        struct seg_desc cs_info;
        if (!cs_offset){
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }

        if (cpu_seg_load_descriptor(cs, &cs_info, 13, cs_offset))
            return 1;
        access = ((&cs_info)->raw[1] >> 8 & 0xFFFF);
        rpl    = (cs & 3);
        dpl    = (access >> 5 & 3);
        if (rpl < m_cpl) {
                cpu_exception(13, (cs_offset) | 0x10000);
                return 1;
            }
        switch (((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
            case 0x18 ... 0x1B:
                if (dpl != rpl) {
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    }
                break;
            case 0x1C ... 0x1F:
                if (dpl > rpl) {
                        cpu_exception(13, (cs_offset) | 0x10000);
                        return 1;
                    };
                break;
            default: {
                    cpu_exception(13, (cs_offset) | 0x10000);
                    return 1;
                }
        }
        if ((access & 0x80) == 0) {
                cpu_exception(11, (cs_offset) | 0x10000);
                return 1;
            }

        if (rpl > m_cpl) {
            int      ss_access, ss_rpl, ss_dpl, ss_type;
            DWORD new_ss = 0, new_esp = 0, new_ss_offset, esp_mask;
            STACK_esp = (STACK_esp + adjust) & STACK_esp_mask;
            if (is32) {
                {
                    {
                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                new_esp = m_read_result;
                            else
                                return 1;
                        } else {
                            new_esp = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                }
                {
                    {
                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> STACK_mask, STACK_mask))
                                new_ss = m_read_result;
                            else
                                return 1;
                        } else {
                            new_ss = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    STACK_esp = (STACK_esp + 4) & STACK_esp_mask;
                }
                new_ss &= 0xFFFF;
            } else {
                {
                    {
                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 1)) {
                            if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                new_esp = m_read_result;
                            else
                                return 1;
                        } else {
                            new_esp = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                }
                {
                    {
                        DWORD addr_ = STACK_esp + STACK_ss_base, shift_ = STACK_mask, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 1)) {
                            if (!cpu_access_read16(addr_, tag >> STACK_mask, STACK_mask))
                                new_ss = m_read_result;
                            else
                                return 1;
                        } else {
                            new_ss = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                        }
                    }
                    STACK_esp = (STACK_esp + 2) & STACK_esp_mask;
                }
            }
            new_ss_offset = new_ss & 0xFFFC;
            if (!new_ss_offset){
                    cpu_exception(13, (new_ss_offset) | 0x10000);
                    return 1;
                }

            struct seg_desc ss_info;
            if (cpu_seg_load_descriptor(new_ss, &ss_info, 13, new_ss_offset))
                return 1;
            ss_access = ((&ss_info)->raw[1] >> 8 & 0xFFFF);
            ss_dpl    = (ss_access >> 5 & 3);
            ss_rpl    = (new_ss & 3);
            ss_type   = ((ss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
            if (!(ss_type == 0x12 || ss_type == 0x13 || ss_type == 0x16 || ss_type == 0x17) || ss_rpl != rpl ||
                ss_dpl != rpl) {
                 {
                    cpu_exception(13, (new_ss_offset) | 0x10000);
                    return 1;
                }
            }
            if ((ss_access & 0x80) == 0){
                    cpu_exception(11, (new_ss_offset) | 0x10000);
                    return 1;
                }

            if (cpu_seg_load_protected(2, new_ss, &ss_info))
                return 1;
            if (cpu_load_csip_protected(cs, &cs_info, eip))
                return 1;
            esp_mask   = ss_access & 0x4000 ? -1 : 0xFFFF;
            m_reg32[4] = ((new_esp + adjust) & esp_mask) | (m_reg32[4] & ~esp_mask);
        } else {
            if (cpu_load_csip_protected(cs, &cs_info, eip))
                return 1;
            STACK_esp  = (STACK_esp + adjust) & STACK_esp_mask;
            m_reg32[4] = (STACK_esp_mask & STACK_esp) | (STACK_original_esp & ~STACK_esp_mask);
        }
        return 0;
    }
}
void CpuInternal::f_reload_cs_base(void)
{
    DWORD virt_eip = (m_phys_eip + m_eip_phys_bias);
    DWORD lin_page = virt_eip >> 12, shift = m_tlb_shift_read, tag = m_tlb_tags[virt_eip >> 12] >> shift;
    if (tag & 2) {
        m_last_phys_eip = m_phys_eip + 0x1000;
        return;
    }
    m_phys_eip      = m_tlb[lin_page] + virt_eip;
    m_last_phys_eip = m_phys_eip & ~0xFFF;
    m_eip_phys_bias = virt_eip - m_phys_eip;
}
int CpuInternal::f_sysenter(void)
{
    DWORD cs = m_sysenter[0], cs_offset = cs & 0xFFFC;
    if ((m_cr[0] & 1) == 0 || cs_offset == 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        } while (0);
    m_eflags &= ~(0x200 | 0x20000);
    m_phys_eip += (m_sysenter[2]) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_reg32[4]      = m_sysenter[1];
    m_seg[1]        = cs_offset;
    m_seg_base[1]   = 0;
    m_seg_limit[1]  = -1;
    m_seg_access[1] = 0x10 | 0x0B | 0x80 | 0x8000;
    m_cpl           = 0;
    cpu_prot_update_cpl();
    m_state_hash    = 0;
    m_seg[2]        = (cs_offset + 8) & 0xFFFC;
    m_seg_base[2]   = 0;
    m_seg_limit[2]  = -1;
    m_seg_access[2] = 0x10 | 0x03 | 0x80 | 0x8000 | 0x4000;
    m_esp_mask      = -1;
    f_reload_cs_base();
    return 0;
}
int CpuInternal::sysexit(void)
{
    DWORD cs = m_sysenter[0], cs_offset = cs & 0xFFFC;
    if ((m_cr[0] & 1) == 0 || cs_offset == 0 || m_cpl != 0){
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }
    m_phys_eip += (m_reg32[2]) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_reg32[4]      = m_reg32[1];
    m_seg[1]        = (m_sysenter[0] | 3) + 16;
    m_seg_base[1]   = 0;
    m_seg_limit[1]  = -1;
    m_seg_access[1] = 0x10 | 0x0B | 0x80 | 0x8000 | 0x60;
    m_cpl           = 3;
    cpu_prot_update_cpl();
    m_state_hash    = 0;
    m_seg[2]        = (m_sysenter[0] | 3) + 24;
    m_seg_base[2]   = 0;
    m_seg_limit[2]  = -1;
    m_seg_access[2] = 0x10 | 0x03 | 0x80 | 0x8000 | 0x4000 | 0x60;
    m_esp_mask      = -1;
    f_reload_cs_base();
    return 0;
}
