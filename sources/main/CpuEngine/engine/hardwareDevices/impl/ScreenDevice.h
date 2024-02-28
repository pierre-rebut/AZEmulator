//
// Created by pierr on 27/08/2023.
//

#pragma once

#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"
#include "CpuEngine/data/DeviceCreateData.h"

namespace Astra::CPU::Core {

    class ScreenDevice : public HardwareDevice
    {
    protected:
        size_t m_width = 0;
        size_t m_height = 0;
        BYTE* m_data = nullptr;

        std::vector<std::pair<size_t, size_t>> m_addrList;

    public:
        explicit ScreenDevice(const DeviceCreateData& deviceInfo);
        ~ScreenDevice() override;

        LARGE Fetch(DataFormat fmt, size_t address) override;
        void Push(DataFormat, size_t pAddress, LARGE pValue) override;
        int FetchReadOnly(size_t address) const override;

        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override;

        const BYTE* getData() const {return m_data;}
        size_t getWidth() const {return m_width;}
        size_t getHeight() const {return m_height;}

        bool SetScreenSize(size_t width, size_t height);
    };

} // Astra
