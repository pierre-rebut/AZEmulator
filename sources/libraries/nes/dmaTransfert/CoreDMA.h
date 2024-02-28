//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib {

    class CoreDMA : public ICpuCore
    {
    private:
        friend class FactoryDMA;

        union Flags {
            struct {
                bool isTransfer : 1 = false;
                bool isDummy : 1 = true;
                bool oddCycle : 1 = false;
                BYTE unused : 5;
            };

            BYTE reg;
        } flags;

        BYTE dmaPage = 0;
        BYTE dmaAddr = 0;
        BYTE dmaData = 0;
        BYTE oamAddr = 0;

        Ref<IDevice> m_cpuMem = nullptr;

    public:
        bool IsInit() const override { return m_cpuMem.operator bool(); }
        void Reset() override;
        void Execute() override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;
    };

}
