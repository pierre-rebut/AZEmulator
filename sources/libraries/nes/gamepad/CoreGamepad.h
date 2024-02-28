//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/KeyCodes.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib::Nes {

    class CoreGamepad : public ICpuCore
    {
    private:
        friend class Factory;

        BYTE m_controller = 0;
        BYTE m_controllerState = 0;

        Ref<IDevice> m_keyboard = nullptr;

    public:
        bool IsInit() const override { return m_keyboard.operator bool(); }
        void Reset() override;
        void Execute() override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

    private:
        void setControllerKey(KeyCode keyCode);
        void unsetControllerKey(KeyCode keyCode);
    };

}
