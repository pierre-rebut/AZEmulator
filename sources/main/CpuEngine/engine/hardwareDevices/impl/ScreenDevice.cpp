//
// Created by pierr on 27/08/2023.
//

#include "ScreenDevice.h"
#include "CpuEngine/exception/DeviceException.h"

namespace Astra::CPU::Core {
    ScreenDevice::ScreenDevice(const DeviceCreateData& deviceInfo) : HardwareDevice(deviceInfo.uuid, deviceInfo.type, deviceInfo.name) {
        SetScreenSize(deviceInfo.screenData.width, deviceInfo.screenData.height);
        m_status = DeviceStatus::READY;
    }

    ScreenDevice::~ScreenDevice() {
        delete[] m_data;
    }

    LARGE ScreenDevice::Fetch(DataFormat fmt, size_t address) {
        if (address == 0) {
            return m_width & 0xFFFF;
        } else if (address == 2) {
            return m_height & 0xFFFF;
        }

        return 0;
    }

    void ScreenDevice::Push(DataFormat, size_t pAddress, LARGE value) {
        DeviceException::assertV(pAddress < (m_width * m_height * 3), "ScreenDevice: {} exceed max address", pAddress);
        m_data[pAddress] = value & 0xFF;
    }

    const std::vector<std::pair<size_t, size_t>>& ScreenDevice::GetDeviceAddressList() const {
        return m_addrList;
    }

    int ScreenDevice::FetchReadOnly(size_t address) const {
        DeviceException::assertV(address < (m_width * m_height * 3), "ScreenDevice: {} exceed max address", address);
        return (int) m_data[address];
    }

    bool ScreenDevice::SetScreenSize(size_t width, size_t height) {
        if (width < 10 || height < 10 || width > 1024 || height > 720) {
            return false;
        }

        delete[] m_data;

        m_width = width;
        m_height = height;

        m_addrList = {{m_width * m_height * 3, 0}};
        m_data = new BYTE[m_width * m_height * 3];

        return true;
    }

} // Astra