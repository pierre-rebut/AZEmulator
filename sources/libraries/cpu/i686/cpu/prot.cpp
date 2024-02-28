#include "cpu.h"

int CpuInternal::cpu_prot_set_cr(int cr, DWORD v)
{
    DWORD diffxor = v ^ m_cr[cr];
    m_cr[cr]         = v;
    switch (cr) {
        case 0:
            if (diffxor & ((1 << 31) | 1 | 65536))
                cpu_mmu_tlb_flush();
            break;
        case 3:
            m_cr[3] &= ~31;
            if (m_cr[4] & (1 << 7))
                cpu_mmu_tlb_flush_nonglobal();
            else
                cpu_mmu_tlb_flush();
            break;
        case 4:
            if (diffxor & ((1 << 7) | (1 << 5) | (1 << 4) | (1 << 17) | (1 << 20)))
                cpu_mmu_tlb_flush();
    }
    return 0;
}
void CpuInternal::cpu_prot_set_dr(int id, DWORD val)
{
    switch (id) {
        case 0 ... 3:
            m_dr[id] = val;
            cpu_mmu_tlb_invalidate(val);
            break;
        case 6:
            m_dr[6] = (m_dr[6] & 0xffff0ff0) | (val & 0xE00F);
            break;
        case 7:
            m_dr[7] = (val & 0xffff2fff) | 0x400;
            cpu_mmu_tlb_flush();
            break;
        default:
            m_dr[id] = val;
            break;
    }
}
void CpuInternal::cpu_prot_update_cpl(void)
{
    if (m_cpl == 3) {
        m_tlb_shift_read  = 4;
        m_tlb_shift_write = 6;
    } else {
        m_tlb_shift_read  = 0;
        m_tlb_shift_write = 2;
    }
}
