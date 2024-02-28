//
// Created by pierr on 14/01/2024.
//

#include "Core65c02.h"

namespace Astra::CPU::Lib::CPU {

    // Unconditional Branch
    BYTE Core65c02::BRA() {
        addr_abs = pc + addr_rel;

        if ((addr_abs & 0xFF00) != (pc & 0xFF00))
            cycles++;

        pc = addr_abs;

        return 0;
    }

    // Push/pull X to stack
    BYTE Core65c02::PHX() {
        push8(x);
        return 0;
    }

    BYTE Core65c02::PLX() {
        x = pull8();

        zerocalc(x);
        signcalc(x);

        return 0;
    }

    // Push/pull Y to stack
    BYTE Core65c02::PHY() {
        push8(y);
        return 0;
    }

    BYTE Core65c02::PLY() {
        y = pull8();

        zerocalc(y);
        signcalc(y);

        return 0;
    }

    // Store zero to memory.
    BYTE Core65c02::STZ() {
        push(0);
        return 0;
    }

    /*
     * TRB & TSB - Test and Change bits
     */

    BYTE Core65c02::TSB() {
        fetch();                                        // Read memory
        temp = (WORD) a & fetched;                        // calculate A & memory

        zerocalc(temp);                                 // Set Z flag from this.
        temp = fetched | a;                            // Write back value read, A bits are set.

        push(temp & 0xFF);
        return 0;
    }

    BYTE Core65c02::TRB() {
        fetch();                                        // Read memory
        temp = (WORD) a & fetched;                        // calculate A & memory

        zerocalc(temp);                       // Set Z flag from this.
        temp = (WORD) fetched & (a ^ 0xFF);                            // Write back value read, A bits are clear.

        push(temp & 0xFF);
        return 0;
    }

    // *******************************************************************************************
    //
    //                                     BBR and BBS
    //
    // *******************************************************************************************
    void Core65c02::bbr(WORD bitmask) {
        fetch();

        if ((fetched & bitmask) == 0) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
    }

    void Core65c02::bbs(WORD bitmask) {
        fetch();

        if ((fetched & bitmask) != 0) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
    }

    BYTE Core65c02::BBR0() {
        bbr(0x01);
        return 0;
    }

    BYTE Core65c02::BBR1() {
        bbr(0x02);
        return 0;
    }

    BYTE Core65c02::BBR2() {
        bbr(0x04);
        return 0;
    }

    BYTE Core65c02::BBR3() {
        bbr(0x08);
        return 0;
    }

    BYTE Core65c02::BBR4() {
        bbr(0x10);
        return 0;
    }

    BYTE Core65c02::BBR5() {
        bbr(0x20);
        return 0;
    }

    BYTE Core65c02::BBR6() {
        bbr(0x40);
        return 0;
    }

    BYTE Core65c02::BBR7() {
        bbr(0x80);
        return 0;
    }

    BYTE Core65c02::BBS0() {
        bbs(0x01);
        return 0;
    }

    BYTE Core65c02::BBS1() {
        bbs(0x02);
        return 0;
    }

    BYTE Core65c02::BBS2() {
        bbs(0x04);
        return 0;
    }

    BYTE Core65c02::BBS3() {
        bbs(0x08);
        return 0;
    }

    BYTE Core65c02::BBS4() {
        bbs(0x10);
        return 0;
    }

    BYTE Core65c02::BBS5() {
        bbs(0x20);
        return 0;
    }

    BYTE Core65c02::BBS6() {
        bbs(0x40);
        return 0;
    }

    BYTE Core65c02::BBS7() {
        bbs(0x80);
        return 0;
    }

    BYTE Core65c02::RMB0() {
        push(fetch() & ~0x01);
        return 0;
    }

    BYTE Core65c02::RMB1() {
        push(fetch() & ~0x02);
        return 0;
    }

    BYTE Core65c02::RMB2() {
        push(fetch() & ~0x04);
        return 0;
    }

    BYTE Core65c02::RMB3() {
        push(fetch() & ~0x08);
        return 0;
    }

    BYTE Core65c02::RMB4() {
        push(fetch() & ~0x10);
        return 0;
    }

    BYTE Core65c02::RMB5() {
        push(fetch() & ~0x20);
        return 0;
    }

    BYTE Core65c02::RMB6() {
        push(fetch() & ~0x40);
        return 0;
    }

    BYTE Core65c02::RMB7() {
        push(fetch() & ~0x80);
        return 0;
    }

    BYTE Core65c02::SMB0() {
        push(fetch() | 0x01);
        return 0;
    }

    BYTE Core65c02::SMB1() {
        push(fetch() | 0x02);
        return 0;
    }

    BYTE Core65c02::SMB2() {
        push(fetch() | 0x04);
        return 0;
    }

    BYTE Core65c02::SMB3() {
        push(fetch() | 0x08);
        return 0;
    }

    BYTE Core65c02::SMB4() {
        push(fetch() | 0x10);
        return 0;
    }

    BYTE Core65c02::SMB5() {
        push(fetch() | 0x20);
        return 0;
    }

    BYTE Core65c02::SMB6() {
        push(fetch() | 0x40);
        return 0;
    }

    BYTE Core65c02::SMB7() {
        push(fetch() | 0x80);
        return 0;
    }

    BYTE Core65c02::WAI() {
        m_runService->Stop();
        return 0;
    }

    BYTE Core65c02::DBG() {
        m_runService->Breakpoint();
        return 0;
    }
}
