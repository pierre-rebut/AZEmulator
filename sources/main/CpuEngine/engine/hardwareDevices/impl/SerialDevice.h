//
// Created by pierr on 31/10/2023.
//
#pragma once

#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"
#include "CpuEngine/data/DeviceCreateData.h"

#include <queue>
#include <list>

namespace Astra::CPU::Core {

    class SerialDevice : public HardwareDevice
    {
    private:
        std::queue<unsigned char> m_inMessage{};
        int outMsg = 0;
        std::queue<unsigned char> m_outMessage{};

    public:
        explicit SerialDevice(const DeviceCreateData& deviceInfo);

        LARGE Fetch(DataFormat fmt, size_t address) override;
        void Push(DataFormat fmt, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override;

        void SendSerialMessage(const std::string& msg);
        void UpdateList(std::list<std::pair<std::string, bool>>& msgList);
    };

}
