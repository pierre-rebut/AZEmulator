#include "cpu.h"

#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))

void CpuInternal::interrupt_guard(void)
{
    m_cycles += cpu_get_cycles() - m_cycles;
    if (m_cycles_to_run != 1) {
        m_refill_counter = m_cycles_to_run - 2;
        m_cycles_to_run  = 2;
        m_cycle_offset   = 2;
    } else {
        m_cycles_to_run      = 1;
        m_cycle_offset       = 1;
        m_refill_counter     = 0;
        m_interrupts_blocked = 1;
    }
}
DWORD CpuInternal::cpu_get_linaddr(DWORD i, optype j)
{
    DWORD addr = m_reg32[i >> 8 & 15];
    addr += m_reg32[i >> 16 & 15] << (i >> 20 & 3);
    addr += j->disp32;
    return (addr & ((i << 12 & 65536) - 1)) + m_seg_base[i >> 22 & 7];
}
DWORD CpuInternal::cpu_get_virtaddr(DWORD i, optype j)
{
    DWORD addr = m_reg32[i >> 8 & 15];
    addr += m_reg32[i >> 16 & 15] << (i >> 20 & 3);
    addr += j->disp32;
    return (addr & ((i << 12 & 65536) - 1));
}
void CpuInternal::cpu_execute(void)
{
    optype i = cpu_get_trace();
    do {
        i = CALL_MEMBER_FN(*this, i->handler)(i);

        if (!--m_cycles_to_run)
            break;
    } while (1);
}
optype CpuInternal::op_ud_exception(optype i)
{
    (void)(i);
    do {
        cpu_exception(6, 0);
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    } while (0);
}
optype CpuInternal::op_trace_end(optype i)
{
    (void)(i);
    m_cycles_to_run++;
    return cpu_get_trace();
}
optype CpuInternal::op_nop(optype i)
{
    (void)(i);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_jmp_r16(optype i)
{
    DWORD dest = m_reg16[i->flags >> 8 & 15];
    if (dest >= m_seg_limit[1])
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_phys_eip += (dest) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmp_r32(optype i)
{
    DWORD dest = m_reg32[i->flags >> 8 & 15];
    if (dest >= m_seg_limit[1])
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_phys_eip += (dest) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmp_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else{
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else {
            src =  m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }

    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    return cpu_get_trace();
}
optype CpuInternal::op_jmp_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else {
            src =  m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        }
    }

    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    return cpu_get_trace();
}
optype CpuInternal::op_call_r16(optype i)
{
    DWORD flags = i->flags;
    if (cpu_push16(((flags & 15) + (m_phys_eip + m_eip_phys_bias)))) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

    m_phys_eip += (m_reg16[flags >> 8 & 15]) - (m_phys_eip + m_eip_phys_bias);
    return cpu_get_trace();
}
optype CpuInternal::op_call_r32(optype i)
{
    DWORD flags = i->flags;
    if (cpu_push32(((flags & 15) + (m_phys_eip + m_eip_phys_bias)))) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

    m_phys_eip += (m_reg32[flags >> 8 & 15]) - (m_phys_eip + m_eip_phys_bias);
    return cpu_get_trace();
}
optype CpuInternal::op_call_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
   {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else {
            src =  m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        }
    }
    if (cpu_push16(((m_phys_eip + m_eip_phys_bias) + (flags & 15)))) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    return cpu_get_trace();
}
optype CpuInternal::op_call_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push32(((m_phys_eip + m_eip_phys_bias) + (flags & 15))))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmp_rel32(optype i)
{
    m_phys_eip += i->flags + i->imm32;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmp_rel16(optype i)
{
    DWORD virt = (m_phys_eip + m_eip_phys_bias);
    m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmpf(optype i)
{
    if (jmpf(i->imm32, i->disp16, (m_phys_eip + m_eip_phys_bias) + i->flags))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmpf_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip = 0, cs = 0;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (jmpf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_jmpf_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip, cs;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (jmpf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_callf16_ap(optype i)
{
    DWORD eip = i->imm32, cs = i->disp16, eip_after = (m_phys_eip + m_eip_phys_bias) + i->flags;
    if (callf(eip, cs, eip_after, 0))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_callf32_ap(optype i)
{
    DWORD eip = i->imm32, cs = i->disp32, eip_after = (m_phys_eip + m_eip_phys_bias) + i->flags;
    if (callf(eip, cs, eip_after, 1))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_callf_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip = 0, cs = 0;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (callf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15), 0))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_callf_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip, cs;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (callf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15), 1))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_retf16(optype i)
{
    if (retf(i->imm16, 0))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_retf32(optype i)
{
    if (retf(i->imm16, 1))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_iret16(optype i)
{
    if (iret((m_phys_eip + m_eip_phys_bias) + i->flags, 0))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_iret32(optype i)
{
    if (iret((m_phys_eip + m_eip_phys_bias) + i->flags, 1))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_loop_rel16(optype i)
{
    DWORD mask = i->disp32, cond = (m_reg32[1] - 1) & mask, virt = (m_phys_eip + m_eip_phys_bias);
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_loop_rel32(optype i)
{
    DWORD mask = i->disp32, cond = (m_reg32[1] - 1) & mask;
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond) {
        m_phys_eip += i->flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_loopz_rel16(optype i)
{
    DWORD mask = i->disp32, cond = (m_reg32[1] - 1) & mask, virt = (m_phys_eip + m_eip_phys_bias);
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && (m_lr == 0)) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_loopz_rel32(optype i)
{
    DWORD mask = i->disp32, cond = (m_reg32[1] - 1) & mask;
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && (m_lr == 0)) {
        m_phys_eip += i->flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_loopnz_rel16(optype i)
{
    DWORD mask = i->disp32, cond = (m_reg32[1] - 1) & mask, virt = (m_phys_eip + m_eip_phys_bias);
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && !(m_lr == 0)) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_loopnz_rel32(optype i)
{
    DWORD mask = i->disp32, cond = (m_reg32[1] - 1) & mask;
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && !(m_lr == 0)) {
        m_phys_eip += i->flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_jecxz_rel16(optype i)
{
    DWORD virt = (m_phys_eip + m_eip_phys_bias);
    if (!(m_reg32[1] & i->disp32)) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_jecxz_rel32(optype i)
{
    if (!(m_reg32[1] & i->disp32)) {
        m_phys_eip += i->flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_jo16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jo32(optype i)
{
    int flags = i->flags;
    if (cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jno16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jno32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jb16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_cf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jb32(optype i)
{
    int flags = i->flags;
    if (cpu_get_cf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnb16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_cf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnb32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_cf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jz16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (m_lr == 0) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jz32(optype i)
{
    int flags = i->flags;
    if (m_lr == 0) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnz16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!(m_lr == 0)) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnz32(optype i)
{
    int flags = i->flags;
    if (!(m_lr == 0)) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jbe16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if ((m_lr == 0) || cpu_get_cf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jbe32(optype i)
{
    int flags = i->flags;
    if ((m_lr == 0) || cpu_get_cf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnbe16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!((m_lr == 0) || cpu_get_cf())) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnbe32(optype i)
{
    int flags = i->flags;
    if (!((m_lr == 0) || cpu_get_cf())) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_js16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_sf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_js32(optype i)
{
    int flags = i->flags;
    if (cpu_get_sf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jns16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_sf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jns32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_sf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jp16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_pf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jp32(optype i)
{
    int flags = i->flags;
    if (cpu_get_pf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnp16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_pf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnp32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_pf()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jl16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_sf() != cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jl32(optype i)
{
    int flags = i->flags;
    if (cpu_get_sf() != cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnl16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_sf() == cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnl32(optype i)
{
    int flags = i->flags;
    if (cpu_get_sf() == cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jle16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if ((m_lr == 0) || (cpu_get_sf() != cpu_get_of())) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jle32(optype i)
{
    int flags = i->flags;
    if ((m_lr == 0) || (cpu_get_sf() != cpu_get_of())) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnle16(optype i)
{
    int      flags = i->flags;
    DWORD virt  = (m_phys_eip + m_eip_phys_bias);
    if (!(m_lr == 0) && (cpu_get_sf() == cpu_get_of())) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_jnle32(optype i)
{
    int flags = i->flags;
    if (!(m_lr == 0) && (cpu_get_sf() == cpu_get_of())) {
        m_phys_eip += flags + i->imm32;
        do {
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            return i + 1;
        } while (1);
    ;
}
optype CpuInternal::op_call_j16(optype i)
{
    DWORD virt_base = (m_phys_eip + m_eip_phys_bias), virt = virt_base + i->flags;
    if (cpu_push16((virt)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += ((virt + i->imm32) & 0xFFFF) - virt_base;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_call_j32(optype i)
{
    DWORD flags = i->flags, virt = (m_phys_eip + m_eip_phys_bias) + flags;
    if (cpu_push32((virt)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags + i->imm32;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_ret16(optype i)
{
    (void)(i);
    if (cpu_pop16((&utemp.d16)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += (utemp.d16) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_ret32(optype i)
{
    (void)(i);
    if (cpu_pop32((&utemp.d32)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += (utemp.d32) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_ret16_iw(optype i)
{
    (void)(i);
    if (cpu_pop16((&utemp.d16)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += (utemp.d16) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_reg32[4] = ((m_reg32[4] + i->imm16) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_ret32_iw(optype i)
{
    (void)(i);
    if (cpu_pop32((&utemp.d32)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += (utemp.d32) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_reg32[4] = ((m_reg32[4] + i->imm16) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_int(optype i)
{
    if (cpu_interrupt(i->imm8, 0, INTERRUPT_TYPE_SOFTWARE, (m_phys_eip + m_eip_phys_bias) + i->flags))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_into(optype i)
{
    //__asm__("int3");
     m_phys_eip += i->flags;
        return i + 1;
    if (cpu_get_of()) {
        if (cpu_interrupt(4, 0, INTERRUPT_TYPE_SOFTWARE, (m_phys_eip + m_eip_phys_bias) + i->flags))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        do {
            return cpu_get_trace();
        } while (0);
    }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_push_r16(optype i)
{
    int flags = i->flags;
    if (cpu_push16((m_reg16[flags >> 8 & 15])))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_i16(optype i)
{
    int flags = i->flags;
    if (cpu_push16((i->imm16)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_e16(optype i)
{
    int      flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push16((src)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_r32(optype i)
{
    int flags = i->flags;
    if (cpu_push32((m_reg32[flags >> 8 & 15])))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_i32(optype i)
{
    int flags = i->flags;
    if (cpu_push32((i->imm32)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_e32(optype i)
{
    int      flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    DWORD src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push32((src)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pop_r16(optype i)
{
    int flags = i->flags;
    if (cpu_pop16((&utemp.d16)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_reg16[flags >> 8 & 15] = utemp.d16;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pop_e16(optype i)
{
    DWORD prev_esp = m_reg32[4], linaddr, flags = i->flags, temp_esp;
    if (cpu_pop16((&utemp.d16)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    linaddr    = cpu_get_linaddr(flags, i);
    temp_esp   = m_reg32[4];
    m_reg32[4] = prev_esp;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = utemp.d16, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_reg32[4] = temp_esp;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pop_e32(optype i)
{
    DWORD prev_esp = m_reg32[4], linaddr, flags = i->flags, temp_esp;
    if (cpu_pop32((&temp.d32)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    linaddr    = cpu_get_linaddr(flags, i);
    temp_esp   = m_reg32[4];
    m_reg32[4] = prev_esp;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = temp.d32, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_reg32[4] = temp_esp;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pop_r32(optype i)
{
    int flags = i->flags;
    if (cpu_pop32((&temp.d32)))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_reg32[flags >> 8 & 15] = temp.d32;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_s16(optype i)
{
    int flags = i->flags;
    if (cpu_push16((m_seg[flags >> 8 & 15])))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_push_s32(optype i)
{
    int      flags = i->flags;
    DWORD esp = m_reg32[4], esp_mask = m_esp_mask, esp_minus_four = (esp - 4) & esp_mask;
    do {
        DWORD addr_ = esp_minus_four + m_seg_base[2], shift_ = m_tlb_shift_write, data_ = m_seg[flags >> 8 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_reg32[4] = esp_minus_four | (esp & ~esp_mask);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pop_s16(optype i)
{
    int      flags = i->flags, seg_dest = flags >> 8 & 15;
    WORD dest;
    do {
        DWORD addr_ = (m_reg32[4] & m_esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(seg_dest, dest))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg32[4] = ((m_reg32[4] + 2) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    if (seg_dest == 2)
        interrupt_guard();
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pop_s32(optype i)
{
    int      flags = i->flags, seg_dest = flags >> 8 & 15;
    WORD dest;
    do {
        DWORD addr_ = (m_reg32[4] & m_esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(seg_dest, dest))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg32[4] = ((m_reg32[4] + 4) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    if (seg_dest == 2)
        interrupt_guard();
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_pusha(optype i)
{
    if (cpu_pusha())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_pushad(optype i)
{
    if (cpu_pushad())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_popa(optype i)
{
    if (cpu_popa())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_popad(optype i)
{
    if (cpu_popad()) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += i->flags & 15;
        return i + 1;
}
optype CpuInternal::op_arith_r8r8(optype i)
{
    int flags = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_arith8(flags >> 25 & 7, m_reg8[flags >> 8 & 15], m_reg8[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
    return i + 1;
}

optype CpuInternal::op_arith_r8i8(optype i)
{
    int flags = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_arith8(flags >> 25 & 7, m_reg8[flags >> 8 & 15], i->imm8);
        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r8e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    BYTE  res;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                res = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            res = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    }

    m_reg8[flags >> 12 & 15] = cpu_arith8(flags >> 25 & 7, m_reg8[flags >> 12 & 15], res);

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_arith_e8r8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_arith8(flags >> 25 & 7, (BYTE)m_read_result, m_reg8[flags >> 12 & 15]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_arith8(flags >> 25 & 7, val, m_reg8[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_e8i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_arith8(flags >> 25 & 7, (BYTE)m_read_result, i->imm8);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_arith8(flags >> 25 & 7, val, i->imm8);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r16r16(optype i)
{
    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_arith16(flags >> 25 & 7, m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r16i16(optype i)
{
    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_arith16(flags >> 25 & 7, m_reg16[flags >> 8 & 15], i->imm16);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD res;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                res = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            res = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }

    m_reg16[flags >> 12 & 15] = cpu_arith16(flags >> 25 & 7, m_reg16[flags >> 12 & 15], res);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_e16r16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_arith16(flags >> 25 & 7, (WORD)m_read_result, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_arith16(flags >> 25 & 7, val, m_reg16[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_e16i16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_arith16(flags >> 25 & 7, (WORD)m_read_result, i->imm16);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_arith16(flags >> 25 & 7, val, i->imm16);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r32r32(optype i)
{
    int flags = i->flags;

    m_reg32[flags >> 8 & 15] = cpu_arith32(flags >> 25 & 7, m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r32i32(optype i)
{
    int flags = i->flags;

    m_reg32[flags >> 8 & 15] = cpu_arith32(flags >> 25 & 7, m_reg32[flags >> 8 & 15], i->imm32);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    DWORD res;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                res = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            res = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }

    m_reg32[flags >> 12 & 15] = cpu_arith32(flags >> 25 & 7, m_reg32[flags >> 12 & 15], res);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_e32r32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_arith32(flags >> 25 & 7, (DWORD)m_read_result, m_reg32[flags >> 12 & 15]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_arith32(flags >> 25 & 7, val, m_reg32[flags >> 12 & 15]);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arith_e32i32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_arith32(flags >> 25 & 7, (DWORD)m_read_result, i->imm32);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_arith32(flags >> 25 & 7, val, i->imm32);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_r8cl(optype i)
{
    int flags = i->flags;

    m_reg8[flags >> 8 & 15] = cpu_shift8(flags >> 25 & 7, m_reg8[flags >> 8 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_r8i8(optype i)
{
    int flags = i->flags;

    m_reg8[flags >> 8 & 15] = cpu_shift8(flags >> 25 & 7, m_reg8[flags >> 8 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_e8cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shift8(flags >> 25 & 7, (BYTE)m_read_result, m_reg8[4]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_shift8(flags >> 25 & 7, val, m_reg8[4]);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_e8i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shift8(flags >> 25 & 7, (BYTE)m_read_result, i->imm8);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_shift8(flags >> 25 & 7, val, i->imm8);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_r16cl(optype i)
{
    int flags = i->flags;

    m_reg16[flags >> 8 & 15] = cpu_shift16(flags >> 25 & 7, m_reg16[flags >> 8 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_r16i16(optype i)
{
    int flags = i->flags;

    m_reg16[flags >> 8 & 15] = cpu_shift16(flags >> 25 & 7, m_reg16[flags >> 8 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_e16cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shift16(flags >> 25 & 7, (WORD)m_read_result, m_reg8[4]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_shift16(flags >> 25 & 7, val, m_reg8[4]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_e16i16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shift16(flags >> 25 & 7, (WORD)m_read_result, i->imm8);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_shift16(flags >> 25 & 7, val, i->imm8);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_r32cl(optype i)
{
    int flags = i->flags;

    m_reg32[flags >> 8 & 15] = cpu_shift32(flags >> 25 & 7, m_reg32[flags >> 8 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_r32i32(optype i)
{
    int flags = i->flags;

    m_reg32[flags >> 8 & 15] = cpu_shift32(flags >> 25 & 7, m_reg32[flags >> 8 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_e32cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shift32(flags >> 25 & 7, (DWORD)m_read_result, m_reg8[4]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_shift32(flags >> 25 & 7, val, m_reg8[4]);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shift_e32i32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shift32(flags >> 25 & 7, (DWORD)m_read_result, i->imm8);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_shift32(flags >> 25 & 7, val, i->imm8);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
    return i + 1;
}

optype CpuInternal::op_cmp_e8r8(optype i)
{
    DWORD flags = i->flags;
    BYTE  src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    }

    m_lop2 = m_reg8[flags >> 12 & 15];
    m_lr   = (int8_t)(src - m_lop2);
    m_laux = SUB8;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_r8r8(optype i)
{
    DWORD flags = i->flags;
    m_lop2         = m_reg8[flags >> 12 & 15];
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB8;

        m_phys_eip += flags & 15;
        return i + 1;

}

optype CpuInternal::op_cmp_r8e8(optype i)
{
    DWORD flags = i->flags;
    BYTE  src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    }

    m_lop2 = src;
    m_lr   = (int8_t)(m_reg8[flags >> 12 & 15] - src);
    m_laux = SUB8;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_r8i8(optype i)
{
    DWORD flags = i->flags;
    m_lop2         = i->imm8;
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB8;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_e8i8(optype i)
{
    DWORD flags = i->flags;
    BYTE  src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    }

    m_lop2 = i->imm8;
    m_lr   = (int8_t)(src - m_lop2);
    m_laux = SUB8;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_e16r16(optype i)
{
    DWORD flags = i->flags;
    WORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_lop2 = m_reg16[flags >> 12 & 15];
    m_lr   = (int16_t)(src - m_lop2);
    m_laux = SUB16;

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmp_r16r16(optype i)
{
    DWORD flags = i->flags;
    m_lop2         = m_reg16[flags >> 12 & 15];
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB16;

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmp_r16e16(optype i)
{
    DWORD flags = i->flags;
    WORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }

    m_lop2 = src;
    m_lr   = (int16_t)(m_reg16[flags >> 12 & 15] - src);
    m_laux = SUB16;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_r16i16(optype i)
{
    DWORD flags = i->flags;
    m_lop2         = i->imm16;
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB16;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_e16i16(optype i)
{
    DWORD flags = i->flags;
    WORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_lop2 = i->imm16;
    m_lr   = (int16_t)(src - m_lop2);
    m_laux = SUB16;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_e32r32(optype i)
{
    DWORD flags = i->flags;
    DWORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_lop2 = m_reg32[flags >> 12 & 15];
    m_lr   = (int32_t)(src - m_lop2);
    m_laux = SUB32;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_r32r32(optype i)
{
    DWORD flags = i->flags;
    m_lop2         = m_reg32[flags >> 12 & 15];
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB32;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_r32e32(optype i)
{
    DWORD flags = i->flags;
    DWORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_lop2 = src;
    m_lr   = (int32_t)(m_reg32[flags >> 12 & 15] - src);
    m_laux = SUB32;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_r32i32(optype i)
{
    DWORD flags = i->flags;
    m_lop2         = i->imm32;
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB32;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmp_e32i32(optype i)
{
    DWORD flags = i->flags;
    DWORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_lop2 = i->imm32;
    m_lr   = (int32_t)(src - m_lop2);
    m_laux = SUB32;

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_test_e8r8(optype i)
{
    DWORD flags = i->flags;
    BYTE  src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int8_t)(src & m_reg8[flags >> 12 & 15]);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r8r8(optype i)
{
    DWORD flags = i->flags;
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] & m_reg8[flags >> 12 & 15]);
    m_laux         = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r8e8(optype i)
{
    DWORD flags = i->flags;
    BYTE  src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int8_t)(src & m_reg8[flags >> 12 & 15]);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r8i8(optype i)
{
    DWORD flags = i->flags;
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] & i->imm8);
    m_laux         = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_e8i8(optype i)
{
    DWORD flags = i->flags;
    BYTE  src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int8_t)(i->imm8 & src);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_e16r16(optype i)
{
    DWORD flags = i->flags;
    WORD src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int16_t)(src & m_reg16[flags >> 12 & 15]);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r16r16(optype i)
{
    DWORD flags = i->flags;
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] & m_reg16[flags >> 12 & 15]);
    m_laux         = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r16e16(optype i)
{
    DWORD flags = i->flags;
    WORD src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int16_t)(src & m_reg16[flags >> 12 & 15]);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r16i16(optype i)
{
    DWORD flags = i->flags;
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] & i->imm16);
    m_laux         = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_e16i16(optype i)
{
    DWORD flags = i->flags;
    WORD src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int16_t)(i->imm16 & src);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_e32r32(optype i)
{
    DWORD flags = i->flags;
    DWORD src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int32_t)(src & m_reg32[flags >> 12 & 15]);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r32r32(optype i)
{
    DWORD flags = i->flags;
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] & m_reg32[flags >> 12 & 15]);
    m_laux         = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r32e32(optype i)
{
    DWORD flags = i->flags;
    DWORD src;
    {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_lr   = (int32_t)(src & m_reg32[flags >> 12 & 15]);
    m_laux = BIT;

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_r32i32(optype i)
{
    DWORD flags = i->flags;
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] & i->imm32);
    m_laux         = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_test_e32i32(optype i)
{
    DWORD flags = i->flags;
    DWORD src;
    do {
        DWORD addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int32_t)(i->imm32 & src);
    m_laux = BIT;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_inc_r8(optype i)
{
    DWORD flags = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_inc8(m_reg8[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_inc_e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_inc8((BYTE)m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_inc8(val);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_inc_r16(optype i)
{
    DWORD flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_inc16(m_reg16[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_inc_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_inc16((WORD)m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_inc16(val);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_inc_r32(optype i)
{
    DWORD flags = i->flags;
    m_reg32[flags >> 8 & 15] = cpu_inc32(m_reg32[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_inc_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_inc32((DWORD)m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_inc32(val);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_dec_r8(optype i)
{
    DWORD flags = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_dec8(m_reg8[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_dec_e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_dec8((BYTE)m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_dec8(val);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_dec_r16(optype i)
{
    DWORD flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_dec16(m_reg16[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_dec_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_dec16((WORD)m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_dec16(val);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_dec_r32(optype i)
{
    DWORD flags = i->flags;

    m_reg32[flags >> 8 & 15] = cpu_dec32(m_reg32[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_dec_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_dec32((DWORD)m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_dec32(val);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_not_r8(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg8[rm] = ~m_reg8[rm];

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_not_e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_not8((BYTE)m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_not8(val);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_not_r16(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg16[rm] = ~m_reg16[rm];

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_not_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_not16((WORD)m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_not16(val);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_not_r32(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg32[rm] = ~m_reg32[rm];

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_not_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_not32((DWORD)m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_not32(val);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_neg_r8(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;

    m_reg8[rm] = cpu_neg8(m_reg8[rm]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_neg_e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_neg8((BYTE)m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_neg8(val);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_neg_r16(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg16[rm] = cpu_neg16(m_reg16[rm]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_neg_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_neg16((WORD)m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_neg16(val);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_neg_r32(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg32[rm] = cpu_neg32(m_reg32[rm]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_neg_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_neg32((DWORD)m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_neg32(val);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_muldiv_r8(optype i)
{
    DWORD flags = i->flags;
    if (cpu_muldiv8(flags >> 25 & 7, m_reg8[flags >> 8 & 15])) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += flags & 15;
    return i + 1;
}

optype CpuInternal::op_muldiv_e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    BYTE  src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    }
    if (cpu_muldiv8(flags >> 25 & 7, src)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_muldiv_r16(optype i)
{
    DWORD flags = i->flags;
    if (cpu_muldiv16(flags >> 25 & 7, m_reg16[flags >> 8 & 15])) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_muldiv_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    if (cpu_muldiv16(flags >> 25 & 7, src)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_muldiv_r32(optype i)
{
    DWORD flags = i->flags;
    if (cpu_muldiv32(flags >> 25 & 7, m_reg32[flags >> 8 & 15])) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_muldiv_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    DWORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    if (cpu_muldiv32(flags >> 25 & 7, src)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r16r16i16(optype i)
{
    DWORD flags            = i->flags;
    m_reg16[flags >> 12 & 15] = cpu_imul16(m_reg16[flags >> 8 & 15], i->imm16);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r16e16i16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_reg16[flags >> 12 & 15] = cpu_imul16(src, i->imm16);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r32r32i32(optype i)
{
    DWORD flags            = i->flags;
    m_reg32[flags >> 12 & 15] = cpu_imul32(m_reg32[flags >> 8 & 15], i->imm32);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r32e32i32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    DWORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_reg32[flags >> 12 & 15] = cpu_imul32(src, i->imm32);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r16r16(optype i)
{
    DWORD flags            = i->flags;
    m_reg16[flags >> 12 & 15] = cpu_imul16(m_reg16[flags >> 12 & 15], m_reg16[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r32r32(optype i)
{
    DWORD flags            = i->flags;
    m_reg32[flags >> 12 & 15] = cpu_imul32(m_reg32[flags >> 12 & 15], m_reg32[flags >> 8 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_reg16[flags >> 12 & 15] = cpu_imul16(m_reg16[flags >> 12 & 15], src);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_imul_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    DWORD src;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_reg32[flags >> 12 & 15] = cpu_imul32(m_reg32[flags >> 12 & 15], src);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_r16r16i8(optype i)
{
    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_shrd16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_r32r32i8(optype i)
{
    int flags = i->flags;
    m_reg32[flags >> 8 & 15] = cpu_shrd32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_r16r16cl(optype i)
{
    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_shrd16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_r32r32cl(optype i)
{
    int flags = i->flags;
    m_reg32[flags >> 8 & 15] = cpu_shrd32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_e16r16i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shrd16((WORD)m_read_result, m_reg16[flags >> 12 & 15], i->imm8);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_shrd16(val, m_reg16[flags >> 12 & 15], i->imm8);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_e32r32i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shrd32((DWORD)m_read_result, m_reg32[flags >> 12 & 15], i->imm8);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_shrd32(val, m_reg32[flags >> 12 & 15], i->imm8);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_e16r16cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shrd16((WORD)m_read_result, m_reg16[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_shrd16(val, m_reg16[flags >> 12 & 15], m_reg8[4]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shrd_e32r32cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shrd32((DWORD)m_read_result, m_reg32[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_shrd32(val, m_reg32[flags >> 12 & 15], m_reg8[4]);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_r16r16i8(optype i)
{
    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_shld16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_r32r32i8(optype i)
{
    int flags = i->flags;
    m_reg32[flags >> 8 & 15] = cpu_shld32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_r16r16cl(optype i)
{
    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = cpu_shld16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_r32r32cl(optype i)
{
    int flags = i->flags;
    m_reg32[flags >> 8 & 15] = cpu_shld32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], m_reg8[4]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_e16r16i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shld16((WORD)m_read_result, m_reg16[flags >> 12 & 15], i->imm8);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_shld16(val, m_reg16[flags >> 12 & 15], i->imm8);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_e32r32i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shld32((DWORD)m_read_result, m_reg32[flags >> 12 & 15], i->imm8);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_shld32(val, m_reg32[flags >> 12 & 15], i->imm8);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_e16r16cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shld16((WORD)m_read_result, m_reg16[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_shld16(val, m_reg16[flags >> 12 & 15], m_reg8[4]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_shld_e32r32cl(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_shld32((DWORD)m_read_result, m_reg32[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_shld32(val, m_reg32[flags >> 12 & 15], m_reg8[4]);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_out_i8al(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 1)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    cpu_outb(port, m_reg8[0]);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_out_i8ax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 2)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    cpu_outw(port, m_reg16[0]);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_out_i8eax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 4)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    cpu_outd(port, m_reg32[0]);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_in_i8al(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 1)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg8[0] = cpu_inb(port);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_in_i8ax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 2)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg16[0] = cpu_inw(port);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_in_i8eax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 4)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg32[0] = cpu_ind(port);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_out_dxal(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 1)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

    cpu_outb(port, m_reg8[0]);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_out_dxax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 2)) {
            m_cycles_to_run++;
            return cpu_get_trace();
        }

    cpu_outw(port, m_reg16[0]);

        m_phys_eip += i->flags & 15;
        return i + 1;
}

optype CpuInternal::op_out_dxeax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 4))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    cpu_outd(port, m_reg32[0]);
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_in_dxal(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 1))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg8[0] = cpu_inb(port);
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_in_dxax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 2))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg16[0] = cpu_inw(port);
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_in_dxeax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 4))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_reg32[0] = cpu_ind(port);
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_mov_r8i8(optype i)
{
    int flags               = i->flags;
    m_reg8[flags >> 8 & 15] = i->imm8;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r16i16(optype i)
{
    int flags                = i->flags;
    m_reg16[flags >> 8 & 15] = i->imm16;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r32i32(optype i)
{
    int flags                = i->flags;
    m_reg32[flags >> 8 & 15] = i->imm32;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r8e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg8[flags >> 12 & 15] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg8[flags >> 12 & 15] = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r8r8(optype i)
{
    DWORD flags          = i->flags;
    m_reg8[flags >> 8 & 15] = m_reg8[flags >> 12 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e8r8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg8[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e8i8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = i->imm8, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg16[flags >> 12 & 15] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg16[flags >> 12 & 15] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r16r16(optype i)
{
    DWORD flags           = i->flags;
    m_reg16[flags >> 8 & 15] = m_reg16[flags >> 12 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e16r16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg16[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e16i16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = i->imm16, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg32[flags >> 12 & 15] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg32[flags >> 12 & 15] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r32r32(optype i)
{
    DWORD flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_reg32[flags >> 12 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e32r32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg32[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e32i32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = i->imm32, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_s16r16(optype i)
{
    int flags = i->flags, dest = flags >> 12 & 15;
    if (cpu_load_seg_value_mov(dest, m_reg16[flags >> 8 & 15]))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    if (dest == 2)
        interrupt_guard();
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_s16e16(optype i)
{
    DWORD flags = i->flags, dest = flags >> 12 & 15, linaddr = cpu_get_linaddr(flags, i);
    WORD src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(dest, src))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    if (dest == 2)
        interrupt_guard();
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_e16s16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r16s16(optype i)
{
    DWORD flags           = i->flags;
    m_reg16[flags >> 8 & 15] = m_seg[flags >> 12 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r32s16(optype i)
{
    DWORD flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_seg[flags >> 12 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_eaxm32(optype i)
{
    int flags = i->flags;
    do {
        DWORD addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg32[0] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg32[0] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_axm16(optype i)
{
    int flags = i->flags;
    do {
        DWORD addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg16[0] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg16[0] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_alm8(optype i)
{
    int flags = i->flags;
    do {
        DWORD addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg8[0] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg8[0] = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_m32eax(optype i)
{
    int flags = i->flags;
    do {
        DWORD addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_write, data_ = m_reg32[0],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_m16ax(optype i)
{
    int flags = i->flags;
    do {
        DWORD addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_write, data_ = m_reg16[0],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_m8al(optype i)
{
    int flags = i->flags;
    do {
        DWORD addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_write, data_ = m_reg8[0],
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmov_r16e16(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_cond(flags >> 25 & 15))
        m_reg16[flags >> 12 & 15] = dest;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmov_r16r16(optype i)
{
    int flags = i->flags;
    if (cpu_cond(flags >> 25 & 15))
        m_reg16[flags >> 12 & 15] = m_reg16[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmov_r32e32(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_cond(flags >> 25 & 15))
        m_reg32[flags >> 12 & 15] = dest;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmov_r32r32(optype i)
{
    int flags = i->flags;
    if (cpu_cond(flags >> 25 & 15))
        m_reg32[flags >> 12 & 15] = m_reg32[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_setcc_e8(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = cpu_cond(flags >> 25 & 15),
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_setcc_r8(optype i)
{
    int flags               = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_cond(flags >> 25 & 15);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lea_r16e16(optype i)
{
    DWORD flags            = i->flags;
    m_reg16[flags >> 12 & 15] = cpu_get_virtaddr(flags, i);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lea_r32e32(optype i)
{
    DWORD flags            = i->flags;
    m_reg32[flags >> 12 & 15] = cpu_get_virtaddr(flags, i);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lds_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(3, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lds_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(3, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_les_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(0, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_les_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(0, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lss_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(2, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lss_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(2, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lfs_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(4, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lfs_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(4, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lgs_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(5, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lgs_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(5, data))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xchg_r8r8(optype i)
{
    DWORD flags           = i->flags;
    BYTE  temp            = m_reg8[flags >> 8 & 15];
    m_reg8[flags >> 8 & 15]  = m_reg8[flags >> 12 & 15];
    m_reg8[flags >> 12 & 15] = temp;
        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xchg_r16r16(optype i)
{
    DWORD flags            = i->flags;
    WORD temp             = m_reg16[flags >> 8 & 15];
    m_reg16[flags >> 8 & 15]  = m_reg16[flags >> 12 & 15];
    m_reg16[flags >> 12 & 15] = temp;
        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xchg_r32r32(optype i)
{
    DWORD flags            = i->flags;
    DWORD temp             = m_reg32[flags >> 8 & 15];
    m_reg32[flags >> 8 & 15]  = m_reg32[flags >> 12 & 15];
    m_reg32[flags >> 12 & 15] = temp;
        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xchg_r8e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int      tlb_info = m_tlb_tags[linaddr >> 12];

    if ((tlb_info >> m_tlb_shift_write & 1)) {
        if (cpu_access_read8(linaddr, tlb_info, m_tlb_shift_write)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }
        cpu_access_write8(linaddr, m_reg8[flags >> 12 & 15], tlb_info, m_tlb_shift_write);
        m_reg8[flags >> 12 & 15] = m_read_result;
    } else {
        BYTE tmp = m_ram->Fetch(DataFormat::Byte, m_tlb[linaddr >> 12] + linaddr);
        m_ram->Push(DataFormat::DWord, m_tlb[linaddr >> 12] + linaddr, m_reg8[flags >> 12 & 15]);
        m_reg8[flags >> 12 & 15] = tmp;
    }

        m_phys_eip += flags & 15;
        return i + 1;

}
optype CpuInternal::op_xchg_r16e16(optype i)
{
    DWORD  flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int       tlb_info = m_tlb_tags[linaddr >> 12];

    if (((linaddr | tlb_info >> m_tlb_shift_write) & 1)) {
        tlb_info >>= m_tlb_shift_write;
        if (cpu_access_read16(linaddr, tlb_info, m_tlb_shift_write)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }
        cpu_access_write16(linaddr, m_reg16[flags >> 12 & 15], tlb_info, m_tlb_shift_write);
        m_reg16[flags >> 12 & 15] = m_read_result;
    } else {
        WORD tmp = m_ram->Fetch(DataFormat::Word, m_tlb[linaddr >> 12] + linaddr);
        m_ram->Push(DataFormat::Word, m_tlb[linaddr >> 12] + linaddr, m_reg16[flags >> 12 & 15]);
        m_reg16[flags >> 12 & 15] = tmp;
    }

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xchg_r32e32(optype i)
{
    DWORD  flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int       tlb_info = m_tlb_tags[linaddr >> 12];

    if (((linaddr | tlb_info >> m_tlb_shift_write) & 3)) {
        tlb_info >>= m_tlb_shift_write;
        if (cpu_access_read32(linaddr, tlb_info, m_tlb_shift_write)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }
        cpu_access_write32(linaddr, m_reg32[flags >> 12 & 15], tlb_info, m_tlb_shift_write);
        m_reg32[flags >> 12 & 15] = m_read_result;
    } else {
        DWORD tmp = m_ram->Fetch(DataFormat::DWord, m_tlb[linaddr >> 12] + linaddr);
        m_ram->Push(DataFormat::DWord, m_tlb[linaddr >> 12] + linaddr, m_reg32[flags >> 12 & 15]);
        m_reg32[flags >> 12 & 15] = tmp;
    }

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cmpxchg_r8r8(optype i)
{
    DWORD flags = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_cmpxchg8(m_reg8[flags >> 8 & 15], m_reg8[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmpxchg_e8r8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_cmpxchg8((BYTE)m_read_result, m_reg8[flags >> 12 & 15]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Byte, addr);
        val = cpu_cmpxchg8(val, m_reg8[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmpxchg_r16r16(optype i)
{
    DWORD flags = i->flags;

    m_reg16[flags >> 8 & 15] = cpu_cmpxchg16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmpxchg_e16r16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_cmpxchg16((WORD)m_read_result, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = cpu_cmpxchg16(val, m_reg16[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmpxchg_r32r32(optype i)
{
    DWORD flags = i->flags;
    m_reg32[flags >> 8 & 15] = cpu_cmpxchg32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmpxchg_e32r32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = cpu_cmpxchg32((DWORD)m_read_result, m_reg32[flags >> 12 & 15]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = cpu_cmpxchg32(val, m_reg32[flags >> 12 & 15]);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_cmpxchg8b_e32(optype i)
{
    DWORD flags = i->flags, low64, high64, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                low64 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            low64 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_write, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                high64 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            high64 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (m_reg32[0] == low64 && m_reg32[2] == high64) {
        cpu_set_zf(1);
        do {
            DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg32[3], tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    do {
                        m_cycles_to_run++;
                        return cpu_get_trace();
                    } while (0);
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        do {
            DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = m_reg32[1], tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    do {
                        m_cycles_to_run++;
                        return cpu_get_trace();
                    } while (0);
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
    } else {
        cpu_set_zf(0);
        m_reg32[0] = low64;
        m_reg32[2] = high64;
    }

        m_phys_eip += flags & 15;
        return i + 1;

}

optype CpuInternal::op_xadd_r8r8(optype i)
{
    DWORD flags = i->flags;
    xadd8(m_reg8[flags >> 8 & 15], m_reg8[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_xadd_r8e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        BYTE res = m_read_result;
        xadd8(res, m_reg8[flags >> 12 & 15]);
        cpu_access_write8(linaddr, res, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        BYTE val = m_ram->Fetch(DataFormat::Byte, addr);
        xadd8(val, m_reg8[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Byte, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_xadd_r16r16(optype i)
{
    DWORD flags = i->flags;
    xadd16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_xadd_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        WORD res = m_read_result;
        xadd16(res, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, res, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        WORD val = m_ram->Fetch(DataFormat::Word, addr);
        xadd16(val, m_reg16[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_xadd_r32r32(optype i)
{
    DWORD flags = i->flags;
    xadd32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xadd_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)){
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        DWORD res = m_read_result;
        xadd32(res, m_reg32[flags >> 12 & 15]);
        cpu_access_write32(linaddr, res, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        DWORD val = m_ram->Fetch(DataFormat::DWord, addr);
        xadd32(val, m_reg32[flags >> 12 & 15]);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bound_r16e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int16_t  index16 = m_reg16[flags >> 12 & 15], low, hi;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                low = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            low = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                hi = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            hi = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (index16 < low || index16 > hi)
        do {
            cpu_exception(5, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bound_r32e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int32_t  index32 = m_reg32[flags >> 12 & 15], low, hi;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                low = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            low = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                hi = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            hi = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (index32 < low || index32 > hi)
        do {
            cpu_exception(5, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_daa(optype i)
{
    BYTE old_al = m_reg8[0], old_cf = cpu_get_cf(), al = old_al;
    int     cond = (al & 0x0F) > 9 || cpu_get_af();
    if (cond) {
        al += 6;
    }
    cpu_set_af(cond);
    cond = (old_al > 0x99) || old_cf;
    if (cond)
        al += 0x60;
    cpu_set_cf(cond);
    m_lr      = (int8_t)al;
    m_reg8[0] = al;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_das(optype i)
{
    BYTE old_al = m_reg8[0], old_cf = cpu_get_cf(), cf = 0, al = old_al;
    int     cond = (old_al & 0x0F) > 9 || cpu_get_af();
    if (cond) {
        al -= 6;
        cf = old_cf | (al > old_al);
    }
    cpu_set_af(cond);
    if (old_al > 0x99 || old_cf == 1) {
        al -= 0x60;
        cf = 1;
    }
    cpu_set_cf(cf);
    m_lr      = (int8_t)al;
    m_reg8[0] = al;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_aaa(optype i)
{
    int cond = 0;
    if ((m_reg8[0] & 15) > 9 || cpu_get_af()) {
        m_reg16[0] += 0x106;
        cond = 1;
    }
    m_reg8[0] &= 15;
    m_laux = BIT;
    m_lr   = (int8_t)m_reg8[0];
    cpu_set_af(cond);
    cpu_set_cf(cond);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_aas(optype i)
{
    int cond = (m_reg8[0] & 0x0F) > 9 || cpu_get_af();
    if (cond) {
        m_reg16[0] -= 6;
        m_reg8[1] -= 1;
    }
    m_reg8[0] &= 0x0F;
    m_laux = BIT;
    m_lr   = (int8_t)m_reg8[0];
    cpu_set_af(cond);
    cpu_set_cf(cond);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_aam(optype i)
{
    if (i->imm8 == 0)
        do {
            cpu_exception(0, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    BYTE temp_al = m_reg8[0];
    m_reg8[1]       = temp_al / i->imm8;
    m_lr            = (int8_t)(m_reg8[0] = temp_al % i->imm8);
    m_laux          = BIT;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_aad(optype i)
{
    BYTE temp_al = m_reg8[0];
    BYTE temp_ah = m_reg8[1];
    m_lr            = (int8_t)(m_reg8[0] = ((temp_al + (temp_ah * i->imm8)) & 0xFF));
    m_reg8[1]       = 0;
    m_laux          = BIT;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_bt_r16(optype i)
{
    DWORD flags = i->flags;
    bt16(m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_bts_r16(optype i)
{
    DWORD flags = i->flags;
    m_reg16[flags >> 8 & 15] = bts16(m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_btc_r16(optype i)
{
    DWORD flags = i->flags;
    m_reg16[flags >> 8 & 15] = btc16(m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_btr_r16(optype i)
{
    DWORD flags = i->flags;
    m_reg16[flags >> 8 & 15] = btr16(m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_bt_r32(optype i)
{
    DWORD flags = i->flags;
    bt32(m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_bts_r32(optype i)
{
    DWORD flags = i->flags;
    m_reg32[flags >> 8 & 15] = bts32(m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_btc_r32(optype i)
{
    DWORD flags = i->flags;
    m_reg32[flags >> 8 & 15] = btc32(m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_btr_r32(optype i)
{
    DWORD flags = i->flags;
    m_reg32[flags >> 8 & 15] = btr32(m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_bt_e16(optype i)
{
    DWORD flags = i->flags, x = (flags & (1 << 25)) ? i->imm8 : m_reg16[flags >> 12 & 15],
             linaddr = cpu_get_linaddr(flags, i), dest;
    {
        DWORD addr_ = linaddr + ((x / 16) * 2), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }

    bt16(dest, x);

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_bt_e32(optype i)
{
    DWORD flags = i->flags, x = (flags & (1 << 25)) ? i->imm8 : m_reg32[flags >> 12 & 15],
             linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        DWORD addr_ = linaddr + ((x / 32) * 4), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    bt32(dest, x);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bts_e16(optype i)
{
    DWORD x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg16[i->flags >> 12 & 15];
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 16) * 2),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = bts16((WORD)m_read_result, x);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = bts16(val, x);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_btc_e16(optype i)
{
    DWORD x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg16[i->flags >> 12 & 15];
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 16) * 2),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = btc16((WORD)m_read_result, x);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = btc16(val, x);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_btr_e16(optype i)
{
    DWORD x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg16[i->flags >> 12 & 15];
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 16) * 2),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = btr16((WORD)m_read_result, x);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = btr16(val, x);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_bts_e32(optype i)
{
    DWORD x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg32[i->flags >> 12 & 15];
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 32) * 4),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = bts32((DWORD)m_read_result, x);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = bts32(val, x);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_btc_e32(optype i)
{
    DWORD x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg32[i->flags >> 12 & 15];
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 32) * 4),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = btc32((DWORD)m_read_result, x);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = btc32(val, x);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
    return i + 1;
}

optype CpuInternal::op_btr_e32(optype i)
{
    DWORD x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg32[i->flags >> 12 & 15];
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 32) * 4),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = btr32((DWORD)m_read_result, x);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::DWord, addr);
        val = btr32(val, x);
        m_ram->Push(DataFormat::DWord, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsf_r16r16(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = bsf16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsf_r16e16(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                }
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    }
    m_reg16[flags >> 12 & 15] = bsf16(data, m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsf_r32r32(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = bsf32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsf_r32e32(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = bsf32(data, m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsr_r16r16(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = bsr16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsr_r16e16(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = bsr16(data, m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsr_r32r32(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = bsr32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bsr_r32e32(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = bsr32(data, m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_cli(optype i)
{
    if ((unsigned int)m_cpl > (m_eflags >> 12 & 3)) {
        if (m_cr[4] & (1 << 0))
            m_eflags &= ~0x80000;
        else
            do {
                cpu_exception(13, (0) | 0x10000);
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            } while (0);
    } else
        m_eflags &= ~0x200;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_sti(optype i)
{
    if ((unsigned int)m_cpl > (m_eflags >> 12 & 3)) {
        if (m_cr[4] & (1 << 0) && !(m_eflags & 0x100000))
            m_eflags |= 0x80000;
        else
            do {
                cpu_exception(13, (0) | 0x10000);
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            } while (0);
    } else
        m_eflags |= 0x200;
    interrupt_guard();
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cld(optype i)
{
    m_eflags &= ~0x400;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_std(optype i)
{
    m_eflags |= 0x400;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cmc(optype i)
{
    cpu_set_cf(cpu_get_cf() ^ 1);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_clc(optype i)
{
    cpu_set_cf(0);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_stc(optype i)
{
    cpu_set_cf(1);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_hlt(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_cycles += cpu_get_cycles() - m_cycles;
    m_cycles_to_run      = 1;
    m_cycle_offset       = 1;
    m_refill_counter     = 0;
    m_interrupts_blocked = 0;
    m_exit_reason        = 3;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cpuid(optype i)
{
    cpuid();
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_rdmsr(optype i)
{
    if (rdmsr(m_reg32[1], &m_reg32[2], &m_reg32[0]))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_wrmsr(optype i)
{
    if (wrmsr(m_reg32[1], m_reg32[2], m_reg32[0]))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_rdtsc(optype i)
{
    if (!(m_cr[4] & (1 << 2)) || (m_cpl == 0) || !(m_cr[0] & 1)) {
        LARGE tsc = cpu_get_cycles() - m_tsc_fudge;
        m_reg32[0]   = tsc;
        m_reg32[2]   = tsc >> 32;
    } else
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_pushf(optype i)
{
    if (pushf())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_pushfd(optype i)
{
    if (pushfd())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_popf(optype i)
{
    if (popf())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_popfd(optype i)
{
    if (popfd())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cbw(optype i)
{
    m_reg16[0] = (int8_t)m_reg8[0];
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cwde(optype i)
{
    m_reg32[0] = (int16_t)m_reg16[0];
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cwd(optype i)
{
    m_reg16[4] = (int16_t)m_reg16[0] >> 15;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_cdq(optype i)
{
    m_reg32[2] = (int32_t)m_reg32[0] >> 31;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_lahf(optype i)
{
    m_reg8[1] = cpu_get_eflags();
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_sahf(optype i)
{
    cpu_set_eflags(m_reg8[1] | (cpu_get_eflags() & ~0xFF));
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_enter16(optype i)
{
    WORD alloc_size    = i->imm16;
    BYTE  nesting_level = i->disp8;
    DWORD frame_temp, ebp, res;
    nesting_level &= 0x1F;
    if (cpu_push16((m_reg16[10])))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    frame_temp = m_reg16[8];
    if (nesting_level != 0) {
        ebp = m_reg32[5];
        while (nesting_level > 1) {
            ebp = ((ebp - 2) & m_esp_mask) | (m_reg32[5] & ~m_esp_mask);
            res = 0;
            do {
                DWORD addr_ = m_reg16[10] + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        res = m_read_result;
                    else
                        do {
                            m_cycles_to_run++;
                            return cpu_get_trace();
                        } while (0);
                } else
                    res = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            if (cpu_push16((res)))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            ;
            nesting_level--;
        }
        if (cpu_push16((frame_temp)))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        ;
    }
    m_reg16[10] = frame_temp;
    m_reg16[8] -= alloc_size;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_enter32(optype i)
{
    WORD alloc_size    = i->imm16;
    BYTE  nesting_level = i->disp8;
    DWORD frame_temp, ebp, res;
    nesting_level &= 0x1F;
    if (cpu_push32((m_reg32[5])))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    ;
    frame_temp = m_reg32[4];
    if (nesting_level != 0) {
        ebp = m_reg32[5];
        while (nesting_level > 1) {
            ebp = ((ebp - 4) & m_esp_mask) | (m_reg32[5] & ~m_esp_mask);
            res = 0;
            do {
                DWORD addr_ = m_reg32[5] + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        res = m_read_result;
                    else
                        do {
                            m_cycles_to_run++;
                            return cpu_get_trace();
                        } while (0);
                } else
                    res = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            if (cpu_push32((res)))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            ;
            nesting_level--;
        }
        if (cpu_push32((frame_temp)))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        ;
    }
    m_reg32[5] = frame_temp;
    m_reg32[4] -= alloc_size;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_leave16(optype i)
{
    DWORD ebp = m_reg32[5], ss_ebp = (ebp & m_esp_mask) + m_seg_base[2];
    do {
        DWORD addr_ = ss_ebp, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg16[10] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg16[10] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((ebp + 2) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_leave32(optype i)
{
    DWORD ebp = m_reg32[5], ss_ebp = (ebp & m_esp_mask) + m_seg_base[2];
    do {
        DWORD addr_ = ss_ebp, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg32[5] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg32[5] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((ebp + 4) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_sgdt_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg_limit[7], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_write, data_ = m_seg_base[7], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_sidt_e32(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg_limit[9], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_write, data_ = m_seg_base[9], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_str_sldt_e16(optype i)
{
    if (m_cr[4] & (1 << 11) && m_cpl > 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg[i->imm8], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_str_sldt_r16(optype i)
{
    if (m_cr[4] & (1 << 11) && m_cpl > 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags           = i->flags;
    m_reg32[flags >> 8 & 15] = (m_seg[i->imm8] & i->disp32) | (m_reg32[flags >> 8 & 15] & ~i->disp32);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lgdt_e16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    WORD limit;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    base &= 0x00FFFFFF;
    m_seg_limit[7] = limit;
    m_seg_base[7]  = base;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lgdt_e32(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    WORD limit;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_seg_limit[7] = limit;
    m_seg_base[7]  = base;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lidt_e16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    WORD limit;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    base &= 0x00FFFFFF;
    m_seg_limit[9] = limit;
    m_seg_base[9]  = base;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lidt_e32(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    WORD limit;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        DWORD addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_seg_limit[9] = limit;
    m_seg_base[9]  = base;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_smsw_r16(optype i)
{
    DWORD flags           = i->flags;
    m_reg16[flags >> 8 & 15] = m_cr[0];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_smsw_r32(optype i)
{
    DWORD flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_cr[0];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_smsw_e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_cr[0], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lmsw_r16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, v = (m_cr[0] & ~0xF) | (m_reg16[flags >> 8 & 15] & 0xF);
    cpu_prot_set_cr(0, v);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lmsw_e16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), v;
    WORD src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_write, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    v = (m_cr[0] & ~0xFFFF) | (src & 0xFFFF);
    cpu_prot_set_cr(0, v);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_invlpg_e8(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags;
    cpu_mmu_tlb_invalidate(cpu_get_linaddr(flags, i));
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_r32cr(optype i)
{
    DWORD flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_cr[flags >> 12 & 15];
    m_phys_eip += flags & 15;
    m_last_phys_eip = m_phys_eip - 4096;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_mov_crr32(optype i)
{
    DWORD flags = i->flags;
    cpu_prot_set_cr(flags >> 12 & 15, m_reg32[flags >> 8 & 15]);
    m_phys_eip += flags & 15;
    m_last_phys_eip = m_phys_eip - 4096;
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_mov_r32dr(optype i)
{
    DWORD flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_dr[flags >> 12 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mov_drr32(optype i)
{
    DWORD flags = i->flags;
    cpu_prot_set_dr(flags >> 12 & 15, m_reg32[flags >> 8 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_ltr_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tr;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                tr = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            tr = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (ltr(tr))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_ltr_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags;
    if (ltr(m_reg32[flags >> 8 & 15] & 0xFFFF))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lldt_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), ldtr;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                ldtr = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            ldtr = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (lldt(ldtr))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lldt_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags;
    if (lldt(m_reg32[flags >> 8 & 15] & 0xFFFF))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lar_r16e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = lar(op1, m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lar_r32e32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = lar(op1, m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lar_r16r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags            = i->flags;
    m_reg16[flags >> 12 & 15] = lar(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lar_r32r32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags            = i->flags;
    m_reg32[flags >> 12 & 15] = lar(m_reg32[flags >> 8 & 15] & 0xFFFF, m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lsl_r16e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = lsl(op1, m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lsl_r32e32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = lsl(op1, m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lsl_r16r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags            = i->flags;
    m_reg16[flags >> 12 & 15] = lsl(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_lsl_r32r32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags            = i->flags;
    m_reg32[flags >> 12 & 15] = lsl(m_reg32[flags >> 8 & 15] & 0xFFFF, m_reg32[flags >> 12 & 15]);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_arpl_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000) {
            cpu_exception(6, 0);

                m_cycles_to_run++;
                return cpu_get_trace();

        }

    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;

    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift)) {
                m_cycles_to_run++;
                return cpu_get_trace();
            }

        m_read_result = arpl((WORD)m_read_result, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        auto addr = m_tlb[linaddr >> 12] + linaddr;
        auto val = m_ram->Fetch(DataFormat::Word, addr);
        val = arpl(val, m_reg16[flags >> 12 & 15]);
        m_ram->Push(DataFormat::Word, addr, val);
    }

        m_phys_eip += flags & 15;
        return i + 1;
}

optype CpuInternal::op_arpl_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000) {
            cpu_exception(6, 0);

                m_cycles_to_run++;
                return cpu_get_trace();
        }

    int flags = i->flags;
    m_reg16[flags >> 8 & 15] = arpl(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);

        m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_verr_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), temp;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            temp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    verify_segment_access(temp, 0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_verr_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags;
    verify_segment_access(m_reg16[flags >> 8 & 15], 0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_verw_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), temp;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            temp = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    verify_segment_access(temp, 1);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_verw_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    DWORD flags = i->flags;
    verify_segment_access(m_reg16[flags >> 8 & 15], 1);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_clts(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_cr[0] &= ~8;
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_wbinvd(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_prefetchh(optype i)
{
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_movzx_r16r8(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = m_reg8[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movzx_r32r8(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = m_reg8[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movzx_r32r16(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = m_reg32[flags >> 8 & 15] & 0xFFFF;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movzx_r16e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src = 0;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movzx_r32e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src = 0;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movzx_r32e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src = 0;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movsx_r16r8(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = (int8_t)m_reg8[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movsx_r32r8(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = (int8_t)m_reg8[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movsx_r32r16(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = (int16_t)m_reg32[flags >> 8 & 15];
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movsx_r16e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = (int8_t)src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movsx_r32e8(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = (int8_t)src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_movsx_r32e16(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = (int16_t)src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xlat16(optype i)
{
    DWORD flags = i->flags, linaddr = (m_reg16[6] + m_reg8[0]) & 0xFFFF, src;
    do {
        DWORD addr_ = linaddr + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg8[0] = src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_xlat32(optype i)
{
    DWORD flags = i->flags, linaddr = m_reg32[3] + m_reg8[0], src;
    do {
        DWORD addr_ = linaddr + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg8[0] = src;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bswap_r16(optype i)
{
    int flags                = i->flags;
    m_reg16[flags >> 8 & 15] = 0;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_bswap_r32(optype i)
{
    int      flags           = i->flags;
    DWORD reg             = m_reg32[flags >> 8 & 15];
    reg                      = (reg & 0xFF) << 24 | (reg & 0xFF00) << 8 | (reg & 0xFF0000) >> 8 | reg >> 24;
    m_reg32[flags >> 8 & 15] = reg;
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_fpu_mem(optype i)
{
    int flags = i->flags;
    if (fpu_mem_op(i, cpu_get_virtaddr(flags, i), flags >> 22 & 7))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_fpu_reg(optype i)
{
    int flags = i->flags;
    if (fpu_reg_op(i, i->flags))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_fwait(optype i)
{
    if (m_cr[0] & (4 | 8))
        do {
            cpu_exception(7, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (fpu_fwait())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_sysenter(optype i)
{
    (void)(i);

    if (f_sysenter())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_sysexit(optype i)
{
    (void)(i);
    if (sysexit())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_sse_10_17(optype i)
{
    if (execute_0F10_17(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_28_2F(optype i)
{
    if (execute_0F28_2F(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_38(optype i)
{
    if (execute_0F38(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_6638(optype i)
{
    if (execute_660F38(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_50_57(optype i)
{
    if (execute_0F50_57(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_58_5F(optype i)
{
    if (execute_0F58_5F(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_60_67(optype i)
{
    if (execute_0F60_67(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_68_6F(optype i)
{
    if (execute_0F68_6F(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_70_76(optype i)
{
    if (execute_0F70_76(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_7C_7D(optype i)
{
    if (execute_0F7C_7D(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_7E_7F(optype i)
{
    if (execute_0F7E_7F(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_C2_C6(optype i)
{
    if (execute_0FC2_C6(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_D0_D7(optype i)
{
    if (execute_0FD0_D7(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_D8_DF(optype i)
{
    if (execute_0FD8_DF(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_E0_E7(optype i)
{
    if (execute_0FE0_E7(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_E8_EF(optype i)
{
    if (execute_0FE8_EF(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_F1_F7(optype i)
{
    if (execute_0FF1_F7(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_sse_F8_FE(optype i)
{
    if (execute_0FF8_FE(i))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_ldmxcsr(optype i)
{
    if (cpu_sse_exception())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i), mxcsr;
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                mxcsr = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            mxcsr = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (mxcsr & ~0xFFFF)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_mxcsr = mxcsr;
    cpu_update_mxcsr();
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_stmxcsr(optype i)
{
    if (cpu_sse_exception())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        DWORD addr_ = linaddr, shift_ = m_tlb_shift_read, data_ = m_mxcsr, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
    } while (0);
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_mfence(optype i)
{
    do {
        m_phys_eip += i->flags & 15;
        return i + 1;
    } while (1);
}
optype CpuInternal::op_fxsave(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    if (fpu_fxsave(linaddr))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_fxrstor(optype i)
{
    DWORD flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    if (fpu_fxrstor(linaddr))
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
    m_phys_eip += flags & 15;
        return i + 1;
}
optype CpuInternal::op_emms(optype i)
{
    if (cpu_emms())
        {
            m_cycles_to_run++;
            return cpu_get_trace();
        }
     m_phys_eip += i->flags;
        return i + 1;
}
optype CpuInternal::op_movsb16(optype i)
{
    int flags = i->flags, result = movsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_movsb32(optype i)
{
    int flags = i->flags, result = movsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_movsw16(optype i)
{
    int flags = i->flags, result = movsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_movsw32(optype i)
{
    int flags = i->flags, result = movsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_movsd16(optype i)
{
    int flags = i->flags, result = movsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_movsd32(optype i)
{
    int flags = i->flags, result = movsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_stosb16(optype i)
{
    int flags = i->flags, result = stosb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_stosb32(optype i)
{
    int flags = i->flags, result = stosb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_stosw16(optype i)
{
    int flags = i->flags, result = stosw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_stosw32(optype i)
{
    int flags = i->flags, result = stosw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_stosd16(optype i)
{
    int flags = i->flags, result = stosd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_stosd32(optype i)
{
    int flags = i->flags, result = stosd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_scasb16(optype i)
{
    int flags = i->flags, result = scasb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_scasb32(optype i)
{
    int flags = i->flags, result = scasb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_scasw16(optype i)
{
    int flags = i->flags, result = scasw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_scasw32(optype i)
{
    int flags = i->flags, result = scasw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_scasd16(optype i)
{
    int flags = i->flags, result = scasd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_scasd32(optype i)
{
    int flags = i->flags, result = scasd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_insb16(optype i)
{
    int flags = i->flags, result = insb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_insb32(optype i)
{
    int flags = i->flags, result = insb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_insw16(optype i)
{
    int flags = i->flags, result = insw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_insw32(optype i)
{
    int flags = i->flags, result = insw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_insd16(optype i)
{
    int flags = i->flags, result = insd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_insd32(optype i)
{
    int flags = i->flags, result = insd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_outsb16(optype i)
{
    int flags = i->flags, result = outsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_outsb32(optype i)
{
    int flags = i->flags, result = outsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_outsw16(optype i)
{
    int flags = i->flags, result = outsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_outsw32(optype i)
{
    int flags = i->flags, result = outsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_outsd16(optype i)
{
    int flags = i->flags, result = outsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_outsd32(optype i)
{
    int flags = i->flags, result = outsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_cmpsb16(optype i)
{
    int flags = i->flags, result = cmpsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_cmpsb32(optype i)
{
    int flags = i->flags, result = cmpsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_cmpsw16(optype i)
{
    int flags = i->flags, result = cmpsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_cmpsw32(optype i)
{
    int flags = i->flags, result = cmpsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_cmpsd16(optype i)
{
    int flags = i->flags, result = cmpsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_cmpsd32(optype i)
{
    int flags = i->flags, result = cmpsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_lodsb16(optype i)
{
    int flags = i->flags, result = lodsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_lodsb32(optype i)
{
    int flags = i->flags, result = lodsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_lodsw16(optype i)
{
    int flags = i->flags, result = lodsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_lodsw32(optype i)
{
    int flags = i->flags, result = lodsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_lodsd16(optype i)
{
    int flags = i->flags, result = lodsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CpuInternal::op_lodsd32(optype i)
{
    int flags = i->flags, result = lodsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
