//
// Created by pierr on 21/10/2023.
//
#pragma once

#include "IMapper.h"

#include <vector>

namespace Astra::CPU::Lib {

    class Mapper_001 : public IMapper
    {
    private:
        BYTE nCHRBankSelect4Lo = 0x00;
        BYTE nCHRBankSelect4Hi = 0x00;
        BYTE nCHRBankSelect8 = 0x00;

        BYTE nPRGBankSelect16Lo = 0x00;
        BYTE nPRGBankSelect16Hi = 0x00;
        BYTE nPRGBankSelect32 = 0x00;

        BYTE nLoadRegister = 0x00;
        BYTE nLoadRegisterCount = 0x00;
        BYTE nControlRegister = 0x00;

        MIRROR mirrormode = MIRROR::HORIZONTAL;

        std::vector<BYTE> vRAMStatic;

    public:
        Mapper_001(BYTE prgBanks, BYTE chrBanks);

        void Reset() override;

        bool cpuRead(size_t addr, size_t& mappedAddr, BYTE& res) override;
        bool cpuWrite(size_t addr, size_t& mappedAddr, BYTE res) override;
        bool ppuRead(size_t addr, size_t& mappedAddr) override;
        bool ppuWrite(size_t addr, size_t& mappedAddr) override;

        MIRROR Mirror() override;
    };

}
