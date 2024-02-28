#include "cpu.h"

#include <cstring>

CpuInternal::CpuInternal(Astra::Ref<IDevice>& a, Astra::Ref<IDevice>& b, Astra::Ref<IDevice>& c) :
        m_ram(a), m_ioBus(b), m_mmio(c) {
    temporary_placeholder.handler = &CpuInternal::op_trace_end;
}

CpuInternal::~CpuInternal() {
    delete[] m_smc_has_code;
}

void util_abort() {
    throw std::runtime_error("abort");
}

void CpuInternal::cpu_set_a20(int a20_enabled) {
    DWORD old_a20_mask = m_a20_mask;
    m_a20_mask = -1 ^ (!a20_enabled << 20);
    if (old_a20_mask != m_a20_mask)
        cpu_mmu_tlb_flush();
}

int CpuInternal::cpu_init_mem() {
    for (int i = 0; i < 0x40000; i++) {
        m_ram->Push(DataFormat::Byte, i + 0xC0000, -1);
    }

    m_smc_has_code_length = (m_memory_size + 4095) >> 12;

    delete[] m_smc_has_code;
    m_smc_has_code = new DWORD[m_smc_has_code_length];
    for (int i = 0; i < m_smc_has_code_length; i++) {
        m_smc_has_code[i] = 0;
    }

    return 0;
}

int CpuInternal::cpu_interrupts_masked(void) {
    return m_eflags & 0x200;
}

itick_t CpuInternal::cpu_get_cycles(void) {
    return m_cycles + (m_cycle_offset - m_cycles_to_run);
}

void CpuInternal::Interrupt(bool isNmi, int interruptId) {
    if (m_eflags & 0x200 && !m_interrupts_blocked) {
        cpu_interrupt(interruptId, 0, INTERRUPT_TYPE_HARDWARE, (m_phys_eip + m_eip_phys_bias));
        m_exit_reason = 0;
    }
}

int CpuInternal::cpu_run(int cycles) {
    m_cycle_offset = cycles;
    m_cycles_to_run = cycles;
    m_refill_counter = 0;
    m_hlt_counter = 0;
    LARGE begin = cpu_get_cycles();
    while (1) {
        if (m_exit_reason == 3)
            return 0;
        if (m_interrupts_blocked) {
            m_refill_counter = cycles;
            m_cycles += cpu_get_cycles() - m_cycles;
            m_cycles_to_run = 1;
            m_cycle_offset = 1;
            m_interrupts_blocked = 0;
        }
        cpu_execute();
        m_cycles += cpu_get_cycles() - m_cycles;
        m_cycles_to_run = m_refill_counter;
        m_refill_counter = 0;
        m_cycle_offset = m_cycles_to_run;
        if (!m_cycles_to_run)
            break;
    }
    int cycles_run = cpu_get_cycles() - begin;
    m_cycle_offset = 0;
    return cycles_run;
}

void CpuInternal::cpu_request_fast_return(int reason) {
    (void) (reason);
    m_cycles += cpu_get_cycles() - m_cycles;
    m_refill_counter = m_cycles_to_run - 1;
    m_cycles_to_run = 1;
    m_cycle_offset = 1;
}

void CpuInternal::cpu_cancel_execution_cycle(int reason) {
    m_exit_reason = reason;
    m_cycles += cpu_get_cycles() - m_cycles;
    m_refill_counter = 0;
    m_cycles_to_run = 1;
    m_cycle_offset = 1;
}

int CpuInternal::cpu_get_exit_reason(void) {
    return m_exit_reason;
}

void CpuInternal::cpu_set_break(void) {
}

void CpuInternal::cpu_reset(void) {
    for (int i = 0; i < 8; i++) {
        m_reg32[i] = 0;
        if (i == 0)
            m_cr[0] = 0x60000010;
        else
            m_cr[i] = 0;
        if (i == 1)
            cpu_seg_load_real(1, 0xF000);
        else
            cpu_seg_load_real(i, 0);
        if (i >= 6)
            m_dr[i] = (i == 6) ? 0xFFFF0FF0 : 0x400;
        else
            m_dr[i] = 0;
    }
    m_phys_eip += (0xFFF0) - (m_phys_eip + m_eip_phys_bias);;
    m_cpl = 0;
    cpu_prot_update_cpl();
    m_eflags = 2;
    m_page_attribute_tables = 0x0007040600070406LL;
    //todo if (apic_is_enabled())
        m_apic_base = 0xFEE00900;
    /*else
        m_apic_base = 0;*/
    m_mxcsr = 0x1F80;
    cpu_update_mxcsr();
    memset(m_tlb, 0, sizeof(DWORD) * (1 << 20));
    memset(m_tlb_tags, 0xFF, 1 << 20);
    memset(m_tlb_attrs, 0xFF, 1 << 20);
    cpu_mmu_tlb_flush();
}

int CpuInternal::cpu_apic_connected(void) {
    return /*todo apic_is_enabled() && */(m_apic_base & 0x100);
}

void CpuInternal::cpu_init_dma(DWORD page) {
    cpu_smc_invalidate_page(page);
}
