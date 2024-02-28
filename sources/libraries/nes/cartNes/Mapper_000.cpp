//
// Created by pierr on 20/10/2023.
//
#include "Mapper_000.h"

namespace Astra::CPU::Lib {
    Mapper_000::Mapper_000(BYTE prgBanks, BYTE chrBanks) : IMapper(prgBanks, chrBanks) {}

    bool Mapper_000::cpuRead(size_t addr, size_t& mappedAddr, BYTE& res) {
        mappedAddr = addr & (m_prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }

    bool Mapper_000::cpuWrite(size_t addr, size_t& mappedAddr, BYTE res) {
        mappedAddr = addr & (m_prgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }

    bool Mapper_000::ppuRead(size_t addr, size_t& mappedAddr) {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            mappedAddr = addr;
            return true;
        }

        return false;
    }

    bool Mapper_000::ppuWrite(size_t addr, size_t& mappedAddr) {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            if (m_chrBanks == 0) {
                // Treat as RAM
                mappedAddr = addr;
                return true;
            }
        }

        return false;
    }
}
