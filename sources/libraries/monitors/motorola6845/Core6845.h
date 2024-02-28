//
// Created by pierr on 27/10/2023.
//
#pragma once

#include <array>

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib::Monitors {

    class Core6845 : public ICpuCore
    {
    private:
        friend class Factory;

        int m_vShift = 0;

        int m_index = 0;
        int m_retrace = 0;
        std::array<int, 16> m_registers{};

        std::array<BYTE, 4000> m_screenData{};
        int yIndex = 0;
        int xIndex = 0;
        Ref<IDevice> m_video = nullptr;

    public:
        bool IsInit() const override;
        bool IsComplete() const override;

        void Reset() override;
        void Execute() override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

        int FetchReadOnly(size_t address) const override;
    };

}
