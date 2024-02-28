#include "cpu.h"

int CpuInternal::cpu_io_check_access(DWORD port, int size)
{
    if ((m_cr[0] & 1) && ((m_eflags & 0x20000) || (unsigned int)m_cpl > (m_eflags >> 12 & 3))) {
        WORD        tss = m_seg[6];
        struct seg_desc tss_info;
        int             tss_access, tss_type;
        if (cpu_seg_load_descriptor(tss, &tss_info, 13, 0))
            return 1;
        tss_access = ((&tss_info)->raw[1] >> 8 & 0xFFFF);
        tss_type   = ((tss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
        if (tss_type != AVAILABLE_TSS_386 && tss_type != BUSY_TSS_386) {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            }
        DWORD base = m_seg_base[6], limit = m_seg_limit[6];
        if (limit < 0x67) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }
        DWORD io_offset;
        {
            DWORD addr_ = base + 0x66, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    io_offset = m_read_result;
                else
                    return 1;
            } else {
                io_offset = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }

        if (limit < (io_offset + ((port + size) >> 3))) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }
        int      mask = ((size << 1) - 1) << (port & 7);
        WORD bitmask;
        {
            DWORD addr_ = base + io_offset + (port >> 3), shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    bitmask = m_read_result;
                else
                    return 1;
            } else {
                bitmask = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            }
        }
        int valid = (bitmask & mask) == 0;
        if (!valid) {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        }

    }
    return 0;
}
void CpuInternal::cpu_outb(DWORD port, DWORD data)
{
    m_ioBus->Push(Astra::CPU::DataFormat::Byte, port, data);
}
void CpuInternal::cpu_outw(DWORD port, DWORD data)
{
    m_ioBus->Push(Astra::CPU::DataFormat::Word, port, data);
}
void CpuInternal::cpu_outd(DWORD port, DWORD data)
{
    m_ioBus->Push(Astra::CPU::DataFormat::DWord, port, data);
}
DWORD CpuInternal::cpu_inb(DWORD port)
{
    return m_ioBus->Fetch(Astra::CPU::DataFormat::Byte, port);
}
DWORD CpuInternal::cpu_inw(DWORD port)
{
    return m_ioBus->Fetch(Astra::CPU::DataFormat::Word, port);
}
DWORD CpuInternal::cpu_ind(DWORD port)
{
    return m_ioBus->Fetch(Astra::CPU::DataFormat::DWord, port);
}
