//
// Created by pierr on 27/10/2023.
//
#pragma once

#include <array>

#include "EngineLib/core/ICpuCore.h"

namespace Astra::CPU::Lib::X86 {

    class Core8259 : public ICpuCore
    {
    private:
        friend class Factory;

        bool forceInterrupt = false;
        int m_imr = 0;
        int m_irr = 0;
        int m_isr = 0;
        int m_icwStep = 0;
        std::array<int, 4> m_icw{};

    public:
        bool IsInit() const override { return true; }

        void Reset() override;
        void Execute() override;
        void Interrupt(bool isNmi, int interruptId) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;
    };

}
