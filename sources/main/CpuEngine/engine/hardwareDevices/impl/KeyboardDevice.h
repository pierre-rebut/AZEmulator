//
// Created by pierr on 03/08/2023.
//
#pragma once

#include <queue>

#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"
#include "CpuEngine/data/DeviceCreateData.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Core {

    class KeyboardDevice : public HardwareDevice
    {
    private:
        std::queue<std::pair<KeyCode, bool>> m_keyQueue;

    public:
        explicit KeyboardDevice(const DeviceCreateData& deviceInfo);

        LARGE Fetch(DataFormat fmt, size_t pAddress) override;

        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override;

        void SetControllerKey(KeyCode keyCode);
        void UnsetControllerKey(KeyCode keyCode);
    };

}
