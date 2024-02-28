//
// Created by pierr on 27/10/2023.
//
#pragma once

#include <array>

#include "EngineLib/core/ICpuCore.h"

namespace Astra::CPU::Lib::X86 {

    class Core8237 : public ICpuCore
    {
    private:
        friend class Factory;

        std::array<int, 4> m_addr{};
        std::array<int, 4> m_cnt{};
        std::array<bool, 4> m_flipflop{};

    public:
        bool IsInit() const override {return true;}
        void Reset() override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;
        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;
    };

}
