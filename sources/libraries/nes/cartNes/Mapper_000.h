//
// Created by pierr on 20/10/2023.
//
#pragma once

#include <cstddef>
#include "IMapper.h"

namespace Astra::CPU::Lib {

    class Mapper_000 : public IMapper
    {
    public:
        Mapper_000(BYTE prgBanks, BYTE chrBanks);

        bool cpuRead(size_t addr, size_t& mappedAddr, BYTE& res) override;
        bool cpuWrite(size_t addr, size_t& mappedAddr, BYTE res) override;
        bool ppuRead(size_t addr, size_t& mappedAddr) override;
        bool ppuWrite(size_t addr, size_t& mappedAddr) override;
    };

}
