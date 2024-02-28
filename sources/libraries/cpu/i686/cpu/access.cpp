#include "cpu.h"

int CpuInternal::cpu_access_read8(DWORD addr, DWORD tag, int shift)
{
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }

    auto host_ptr = m_tlb[addr >> 12] + addr;

    if ((host_ptr >= 0xA0000 && host_ptr < 0xC0000) || (host_ptr >= m_memory_size)) {
        m_read_result = m_mmio->Fetch(Astra::CPU::DataFormat::Byte, host_ptr);
        return 0;
    }

    m_read_result = m_ram->Fetch(DataFormat::Byte, host_ptr);
    return 0;
}
int CpuInternal::cpu_access_read16(DWORD addr, DWORD tag, int shift)
{
    if (addr & 1) {
        DWORD res = 0;
        for (int i = 0, j = 0; i < 2; i++, j += 8) {
            if (cpu_access_read8(addr + i, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
            res |= m_read_result << j;
        }
        m_read_result = res;
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }

    auto host_ptr = m_tlb[addr >> 12] + addr;
    if ((host_ptr >= 0xA0000 && host_ptr < 0xC0000) || (host_ptr >= m_memory_size)) {
        m_read_result = m_mmio->Fetch(Astra::CPU::DataFormat::Word, host_ptr);
        return 0;
    }

    m_read_result = m_ram->Fetch(DataFormat::Word, host_ptr);
    return 0;
}
int CpuInternal::cpu_access_read32(DWORD addr, DWORD tag, int shift)
{
    if (addr & 3) {
        DWORD res = 0;
        for (int i = 0, j = 0; i < 4; i++, j += 8) {
            if (cpu_access_read8(addr + i, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
            res |= m_read_result << j;
        }
        m_read_result = res;
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }

    auto host_ptr = m_tlb[addr >> 12] + addr;

    if ((host_ptr >= 0xA0000 && host_ptr < 0xC0000) || (host_ptr >= m_memory_size)) {
        m_read_result = m_mmio->Fetch(Astra::CPU::DataFormat::DWord, host_ptr);
        return 0;
    }

    m_read_result = m_ram->Fetch(DataFormat::DWord, host_ptr);
    return 0;
}
int CpuInternal::cpu_access_write8(DWORD addr, DWORD data, DWORD tag, int shift)
{
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }

    auto host_ptr = m_tlb[addr >> 12] + addr;

    if ((host_ptr >= 0xA0000 && host_ptr < 0x100000) || (host_ptr >= m_memory_size)) {
        m_mmio->Push(Astra::CPU::DataFormat::Byte, host_ptr, data);
        return 0;
    }

    if (cpu_smc_has_code(host_ptr))
        cpu_smc_invalidate(addr, host_ptr);

    m_ram->Push(DataFormat::Byte, host_ptr, data);
    return 0;
}
int CpuInternal::cpu_access_write16(DWORD addr, DWORD data, DWORD tag, int shift)
{
    if (addr & 1) {
        for (int i = 0, j = 0; i < 2; i++, j += 8) {
            if (cpu_access_write8(addr + i, data >> j, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
        }
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }

    auto host_ptr = m_tlb[addr >> 12] + addr;

    if ((host_ptr >= 0xA0000 && host_ptr < 0x100000) || (host_ptr >= m_memory_size)) {
        m_mmio->Push(Astra::CPU::DataFormat::Word, host_ptr, data);
        return 0;
    }
    if (cpu_smc_has_code(host_ptr))
        cpu_smc_invalidate(addr, host_ptr);

    m_ram->Push(DataFormat::Word, host_ptr, data);
    return 0;
}
int CpuInternal::cpu_access_write32(DWORD addr, DWORD data, DWORD tag, int shift)
{
    if (addr & 3) {
        for (int i = 0, j = 0; i < 4; i++, j += 8) {
            if (cpu_access_write8(addr + i, data >> j, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
        }
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }

    auto host_ptr = m_tlb[addr >> 12] + addr;

    if ((host_ptr >= 0xA0000 && host_ptr < 0x100000) || (host_ptr >= m_memory_size)) {
        m_mmio->Push(Astra::CPU::DataFormat::DWord, host_ptr, data);
        return 0;
    }
    if (cpu_smc_has_code(host_ptr))
        cpu_smc_invalidate(addr, host_ptr);

    m_ram->Push(DataFormat::DWord, host_ptr, data);
    return 0;
}
int CpuInternal::cpu_access_verify(DWORD addr, DWORD end, int shift)
{
    DWORD tag;
    if ((addr ^ end) & ~0xFFF) {
        tag = m_tlb_tags[addr >> 12];
        if (tag & 2) {
            if (cpu_mmu_translate(addr, shift))
                return 1;
        }
    } else
        end = addr;
    tag = m_tlb_tags[end >> 12];
    if (tag & 2) {
        if (cpu_mmu_translate(end, shift))
            return 1;
    }
    return 0;
}
BYTE CpuInternal::read8(DWORD lin)
{
    BYTE dest;
    {
        DWORD addr_ = lin, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else {
                printf("Unable to read memory at address %08x\n", lin);
                return 0;
            }
        } else {
            auto addr = m_tlb[addr_ >> 12] + addr_;
            dest = m_ram->Fetch(DataFormat::Byte, addr);
        }
    }
    return dest;
}
WORD CpuInternal::read16(DWORD lin)
{
    WORD dest;
    {
        DWORD addr_ = lin, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else {
                    printf("Unable to read memory at address %08x\n", lin);
                    return 0;
                }
        } else {
            auto addr = m_tlb[addr_ >> 12] + addr_;
            dest = m_ram->Fetch(DataFormat::Word, addr);
        }
    }
    return dest;
}
DWORD CpuInternal::read32(DWORD lin)
{
    DWORD dest;
    {
        DWORD addr_ = lin, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else {
                    printf("Unable to read memory at address %08x\n", lin);
                    return 0;
                }
        } else {
            auto addr = m_tlb[addr_ >> 12] + addr_;
            dest = m_ram->Fetch(DataFormat::DWord, addr);
        }
    }
    return dest;
}

DWORD CpuInternal::lin2phys(DWORD addr)
{
    BYTE tag = m_tlb_tags[addr >> 12];
    if (tag & 2) {
        if (cpu_mmu_translate(addr, 0)) {
            printf("ERROR TRANSLATING ADDRESS %08x\n", addr);
            return 1;
        }
        tag = m_tlb_tags[addr >> 12] >> 0;
    }

    return m_tlb[addr >> 12] + addr;
}
