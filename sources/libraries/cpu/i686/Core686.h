//
// Created by pierr on 09/02/2024.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"
#include "cpu/cpu.h"

namespace Astra::CPU::Lib::CPU {

    class Core686 : public ICpuCore
    {
    private:
        friend class Factory;
        bool m_isInit = false;

        Ref<IDevice> m_ram;
        Ref<IDevice> m_ioBus;
        Ref<IDevice> m_mmio;

        class CpuInternal m_i386{m_ram, m_ioBus, m_mmio};

    public:
        bool IsInit() const override {
            return m_ram && m_ioBus && m_mmio && m_isInit;
        }

        bool IsComplete() const override {
            return ICpuCore::IsComplete();
        }

        void Reset() override;
        void Execute() override;
        void Interrupt(bool isNmi, int interruptId) override;

        std::vector<int> DebugExecute() const override;


    };

}
