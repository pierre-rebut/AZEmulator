//
// Created by pierr on 20/10/2023.
//
#pragma once

#include <cstdint>
#include "EngineLib/data/Types.h"

namespace Astra::CPU::Lib {

    enum class MIRROR : BYTE
    {
        HARDWARE = 0,
        HORIZONTAL = 1,
        VERTICAL = 2,
        ONESCREEN_LO = 3,
        ONESCREEN_HI = 4,
    };

    class IMapper
    {
    protected:
        BYTE m_prgBanks;
        BYTE m_chrBanks;

    public:
        IMapper(BYTE prgBanks, BYTE chrBanks) : m_prgBanks(prgBanks), m_chrBanks(chrBanks) {}
        virtual ~IMapper() = default;

        virtual void Reset() { /* do nothing */ }
        virtual bool cpuRead(size_t addr, size_t& mappedAddr, BYTE& res) = 0;
        virtual bool cpuWrite(size_t addr, size_t& mappedAddr, BYTE res) = 0;
        virtual bool ppuRead(size_t addr, size_t& mappedAddr) = 0;
        virtual bool ppuWrite(size_t addr, size_t& mappedAddr) = 0;

        virtual MIRROR Mirror() { return MIRROR::HARDWARE; }

        virtual void ScanLine() { /* do nothing */ }

        virtual bool irqState() const { return false; }

        virtual void irqClear() { /* do nothing */ }
    };

}
