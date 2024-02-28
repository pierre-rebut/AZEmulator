//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/KeyCodes.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib::Monitors {

    class CoreScreen : public ICpuCore
    {
    private:
        friend class Factory;

        Ref<IDevice> m_video = nullptr;

    public:
        bool IsInit() const override { return m_video.operator bool(); }

        void Push(DataFormat, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;
    };

}
