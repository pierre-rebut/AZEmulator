//
// Created by pierr on 13/01/2024.
//
#include "Via.h"

namespace Astra::CPU::Lib::X16 {

    void Via::init() {
// timer latches, timer counters and SR are not cleared
        for (int i = 0; i < 4; i++) {
            registers[i] = 0;
        }
        for (int i = 11; i < 15; i++) {
            registers[i] = 0;
        }

        timerRunning[0] = false;
        timerRunning[1] = false;
        timer1M1 = false;
        pb7Output = true;
    }

    void Via::clearPraIrqs() {
        registers[13] &= ~0x02;
        if ((registers[12] & 0b00001010) != 0b00000010) {
            registers[13] &= ~0x01;
        }
    }

    void Via::clearPrbIrqs() {
        registers[13] &= ~0x10;
        if ((registers[12] & 0b10100000) != 0b00100000) {
            registers[13] &= ~0x08;
        }
    }

    BYTE Via::read(BYTE reg, bool debug) {
        BYTE ifr;
        bool irq;
        switch (reg) {
            case 0: // IRB
                if (!debug) clearPrbIrqs();
                return registers[0];
            case 1: // IRA
            case 15:
                if (!debug) clearPraIrqs();
                return registers[1];
            case 4: // T1L
                if (!debug) registers[13] &= ~0x40;
                return (BYTE) (timerCount[0] & 0xff);
            case 5: // T1H
                return (BYTE) (timerCount[0] >> 8);
            case 8: // T2L
                if (!debug) registers[13] &= ~0x20;
                return (BYTE) (timerCount[1] & 0xff);
            case 9: // T2H
                return (BYTE) (timerCount[1] >> 8);
            case 10: // SR
                if (!debug) registers[13] &= ~0x04;
                return registers[10];
            case 13: // IFR
                ifr = registers[13];
                irq = (ifr & registers[14]) != 0;
                return ((BYTE) irq << 7) | ifr;
            case 14: // IER
                return registers[14] | 0x80;
            default:
                return registers[reg];
        }
    }

    void Via::write(BYTE reg, BYTE value) {
        BYTE pcr;
        switch (reg) {
            case 0: // ORB
                clearPrbIrqs();
                registers[0] = value;
                break;
            case 1: // ORA
            case 15:
                clearPraIrqs();
                registers[1] = value;
                break;
            case 4: // T1L
                registers[6] = value;
                break;
            case 5: // T1H
            case 7: // T1LH
                registers[13] &= ~0x40;
                registers[7] = value;
                if (reg == 5) {
                    timerCount[0] = ((unsigned) value << 8) | registers[6];
                    timerRunning[0] = true;
                    pb7Output = false;
                }
                break;
            case 9: // T2H
                registers[13] &= ~0x20;
                timerCount[1] = ((unsigned) value << 8) | registers[8];
                timerRunning[1] = true;
                break;
            case 10: // SR
                registers[13] &= ~0x04;
                registers[10] = value;
                break;
            case 13: // IFR
                pcr = registers[12];
                if ((value & 0x01) && ((pcr & 0b00001010) == 0b00000010)) {
                    registers[13] &= ~0x01;
                }
                if ((value & 0x08) && ((pcr & 0b10100000) == 0b00100000)) {
                    registers[13] &= ~0x08;
                }
                break;
            case 14: // IER
                if (value & 0x80) {
                    registers[14] |= value & 0x7f;
                } else {
                    registers[14] &= ~value & 0x7f;
                }
                break;
            default:
                registers[reg] = value;
        }
    }

    void Via::step(unsigned int clocks) {
// TODO there's currently no timestamp mechanism to mark exact transition
        // times, since there's currently no peripherals that require those
        BYTE acr = registers[11];
        BYTE ifr = registers[13];
        // handle timers
        unsigned cnt;
        DWORD tclk, tclk_s, reload;
        // counter always update even if it's not "running"
        cnt = timerCount[0];
        tclk = clocks;
        while (tclk > 0) {
            if (timer1M1) {
                reload = (((DWORD) registers[7] << 8) | registers[6]);
                tclk_s = reload + 1;
                if (tclk < tclk_s) tclk_s = tclk;
                cnt = reload - tclk_s + 1;
                timer1M1 = false;
            } else if (cnt < tclk) {
                if (timerRunning[0]) {
                    ifr |= 0x40;
                    pb7Output ^= true;
                    if (!(acr & 0x40)) timerRunning[0] = false;
                }
                if (tclk - cnt == 1) {
                    // special, -1 state
                    cnt = 0xffff;
                    timer1M1 = true;
                    tclk_s = 1;
                } else {
                    reload = (((DWORD) registers[7] << 8) | registers[6]);
                    tclk_s = cnt + reload + 2;
                    if (tclk < tclk_s) tclk_s = tclk;
                    cnt += reload - tclk_s + 2;
                }
            } else {
                cnt -= tclk;
                break;
            }
            tclk -= tclk_s;
        }
        timerCount[0] = (unsigned) cnt;

        cnt = timerCount[1];
        tclk = (acr & 0x20) ? pb6PulseCounts : clocks;
        pb6PulseCounts = 0;
        if (cnt < tclk) {
            if (timerRunning[1]) {
                ifr |= 0x20;
                timerRunning[1] = false;
            }
            timerCount[1] = 0x10000 + cnt - tclk;
        } else {
            timerCount[1] -= tclk;
        }

        // TODO Cxx pin and shift register handling

        registers[13] = ifr;
    }

    bool Via::isIrq() const {
        return (registers[13] & registers[14]) != 0;
    }

}
