//
// Created by pierr on 21/10/2023.
//
#include "Mapper_001.h"

namespace Astra::CPU::Lib {

    Mapper_001::Mapper_001(BYTE prgBanks, BYTE chrBanks) : IMapper(prgBanks, chrBanks) {
        vRAMStatic.resize(32 * 1024);
    }

    bool Mapper_001::cpuRead(size_t addr, size_t& mappedAddr, BYTE& res) {
        if (nControlRegister & 0b01000) {
            // 16K Mode
            if (addr >= 0x8000 && addr <= 0xBFFF) {
                mappedAddr = nPRGBankSelect16Lo * 0x4000 + (addr & 0x3FFF);
                return true;
            } else {
                mappedAddr = nPRGBankSelect16Hi * 0x4000 + (addr & 0x3FFF);
                return true;
            }
        } else {
            // 32K Mode
            mappedAddr = nPRGBankSelect32 * 0x8000 + (addr & 0x7FFF);
            return true;
        }
    }

    bool Mapper_001::cpuWrite(size_t addr, size_t& mappedAddr, BYTE value) {
        if (value & 0x80) {
            // MSB is set, so Reset serial loading
            nLoadRegister = 0x00;
            nLoadRegisterCount = 0;
            nControlRegister = nControlRegister | 0x0C;
        } else {
            // Load data in serially into load register
            // It arrives LSB first, so implant this at
            // bit 5. After 5 writes, the register is ready
            nLoadRegister >>= 1;
            nLoadRegister |= (value & 0x01) << 4;
            nLoadRegisterCount++;

            if (nLoadRegisterCount == 5) {
                // Get Mapper Target Register, by examining
                // bits 13 & 14 of the address
                BYTE nTargetRegister = (addr >> 13) & 0x03;

                if (nTargetRegister == 0) // 0x8000 - 0x9FFF
                {
                    // Set Control Register
                    nControlRegister = nLoadRegister & 0x1F;

                    switch (nControlRegister & 0x03) {
                        case 0:
                            mirrormode = MIRROR::ONESCREEN_LO;
                            break;
                        case 1:
                            mirrormode = MIRROR::ONESCREEN_HI;
                            break;
                        case 2:
                            mirrormode = MIRROR::VERTICAL;
                            break;
                        case 3:
                            mirrormode = MIRROR::HORIZONTAL;
                            break;
                    }
                } else if (nTargetRegister == 1) { // 0xA000 - 0xBFFF
                    // Set CHR Bank Lo
                    if (nControlRegister & 0b10000) {
                        // 4K CHR Bank at PPU 0x0000
                        nCHRBankSelect4Lo = nLoadRegister & 0x1F;
                    } else {
                        // 8K CHR Bank at PPU 0x0000
                        nCHRBankSelect8 = nLoadRegister & 0x1E;
                    }
                } else if (nTargetRegister == 2) { // 0xC000 - 0xDFFF
                    // Set CHR Bank Hi
                    if (nControlRegister & 0b10000) {
                        // 4K CHR Bank at PPU 0x1000
                        nCHRBankSelect4Hi = nLoadRegister & 0x1F;
                    }
                } else if (nTargetRegister == 3) {  // 0xE000 - 0xFFFF
                    // Configure PRG Banks
                    BYTE nPRGMode = (nControlRegister >> 2) & 0x03;

                    if (nPRGMode == 0 || nPRGMode == 1) {
                        // Set 32K PRG Bank at CPU 0x8000
                        nPRGBankSelect32 = (nLoadRegister & 0x0E) >> 1;
                    } else if (nPRGMode == 2) {
                        // Fix 16KB PRG Bank at CPU 0x8000 to First Bank
                        nPRGBankSelect16Lo = 0;
                        // Set 16KB PRG Bank at CPU 0xC000
                        nPRGBankSelect16Hi = nLoadRegister & 0x0F;
                    } else if (nPRGMode == 3) {
                        // Set 16KB PRG Bank at CPU 0x8000
                        nPRGBankSelect16Lo = nLoadRegister & 0x0F;
                        // Fix 16KB PRG Bank at CPU 0xC000 to Last Bank
                        nPRGBankSelect16Hi = m_prgBanks - 1;
                    }
                }

                // 5 bits were written, and decoded, so
                // Reset load register
                nLoadRegister = 0x00;
                nLoadRegisterCount = 0;
            }
        }

        return false;
    }

    bool Mapper_001::ppuRead(size_t addr, size_t& mappedAddr) {
        if (addr < 0x2000) {
            if (m_chrBanks == 0) {
                mappedAddr = addr;
            } else if (nControlRegister & 0b10000) {
                // 4K CHR Bank Mode
                if (addr <= 0x0FFF) {
                    mappedAddr = nCHRBankSelect4Lo * 0x1000 + (addr & 0x0FFF);
                } else {
                    mappedAddr = nCHRBankSelect4Hi * 0x1000 + (addr & 0x0FFF);
                }
            } else {
                // 8K CHR Bank Mode
                mappedAddr = nCHRBankSelect8 * 0x2000 + (addr & 0x1FFF);
            }

            return true;
        }

        return false;
    }

    bool Mapper_001::ppuWrite(size_t addr, size_t& mappedAddr) {
        if (addr < 0x2000) {
            if (m_chrBanks == 0) {
                mappedAddr = addr;
                return true;
            }
        }

        return false;
    }

    void Mapper_001::Reset() {
        nControlRegister = 0x1C;
        nLoadRegister = 0x00;
        nLoadRegisterCount = 0x00;

        nCHRBankSelect4Lo = 0;
        nCHRBankSelect4Hi = 0;
        nCHRBankSelect8 = 0;

        nPRGBankSelect32 = 0;
        nPRGBankSelect16Lo = 0;
        nPRGBankSelect16Hi = m_prgBanks - 1;
    }

    MIRROR Mapper_001::Mirror() {
        return mirrormode;
    }
}
