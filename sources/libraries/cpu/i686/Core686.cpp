//
// Created by pierr on 09/02/2024.
//
#include "Core686.h"

namespace Astra::CPU::Lib::CPU {
    void Core686::Reset() {
        m_i386.cpu_reset();
        m_i386.fpu_init();
        m_i386.cpu_init_mem();
    }

    void Core686::Execute() {
        m_i386.cpu_run(1);
    }

    void Core686::Interrupt(bool isNmi, int interruptId) {
        m_i386.Interrupt(isNmi, interruptId);
    }

    std::vector<int> Core686::DebugExecute() const {
        /*void CpuInternal::cpu_debug(void) {
            printf("EAX: %08x ECX: %08x EDX: %08x EBX: %08x\n", m_reg32[0], m_reg32[1], m_reg32[2], m_reg32[3]);
            printf("ESP: %08x EBP: %08x ESI: %08x EDI: %08x\n", m_reg32[4], m_reg32[5], m_reg32[6], m_reg32[7]);
            printf("EFLAGS: %08x\n", cpu_get_eflags());
            printf("CS:EIP: %04x:%08x (lin: %08x) Physical EIP: %08x\n", m_seg[1], (m_phys_eip + m_eip_phys_bias),
                   (m_phys_eip + m_eip_phys_bias + m_seg_base[1]), m_phys_eip);
            printf("Translation mode: %d-bit\n", m_state_hash ? 16 : 32);
        }*/
        return ICpuCore::DebugExecute();
    }
}
