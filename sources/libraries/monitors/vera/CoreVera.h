//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/KeyCodes.h"
#include "EngineLib/data/Base.h"

#include "Spi.h"
#include "Vera.h"

namespace Astra::CPU::Lib::Monitors {

    class CoreVera : public ICpuCore
    {
    private:
        friend class Factory;

        bool m_isInit = false;
        Ref<IDevice> m_video = nullptr;
        Ref<IDevice> m_sdCard = nullptr;

        X16Disk sdCardFile{m_sdCard};
        Spi spi{sdCardFile};
        Vera vera{spi};

    public:
        bool UpdateHardParameters(const std::vector<int>& hardParameters) override;

        bool IsInit() const override { return m_isInit && m_video; }

        void Reset() override;

        void Execute() override;

        LARGE Fetch(DataFormat, size_t address) override;

        void Push(DataFormat, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;
    };

}
