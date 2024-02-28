//
// Created by pierr on 27/10/2023.
//
#pragma once

#include <array>

#include "EngineLib/core/ICpuCore.h"

namespace Astra::CPU::Lib::X86 {

    class Core8253 : public ICpuCore
    {
    private:
        friend class Factory;

        std::array<int, 3> m_count{};
        std::array<int, 3> m_value{};
        std::array<int, 3> m_latch{};
        std::array<int, 3> m_control{};
        std::array<bool, 3> m_enabled{};
        std::array<bool, 3> m_latched{};
        std::array<bool, 3> m_outputStatus{};
        std::array<bool, 3> m_toggle{};

    public:
        bool IsInit() const override {return true;}
        void Reset() override;
        void Execute() override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;
        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

    private:
        void output(int sc, bool state);
    };

}
