//
// Created by pierr on 06/02/2024.
//
#pragma once

#include <queue>

#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"
#include "CpuEngine/data/DeviceCreateData.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Core {

    class MouseDevice : public HardwareDevice
    {
    public:
        static constexpr const int MAX_BUFFER_SIZE = 4096;

    private:
        bool isMouseUpdated = false;

        float prevMouseX = 0;
        float prevMouseY = 0;

        BYTE buttons = 0;
        short mouseDiffX = 0;
        short mouseDiffY = 0;
        char wheel = 0;

        std::queue<LARGE> m_mouseQueue;

    public:
        explicit MouseDevice(const DeviceCreateData& deviceInfo);

        LARGE Fetch(DataFormat fmt, size_t pAddress) override;
        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override;

        void SetButtonDown(MouseCode code);
        void SetButtonUp(MouseCode code);
        void SetWheel(float y);
        void SetMove(float x, float y);
        void Update();
    };

}
