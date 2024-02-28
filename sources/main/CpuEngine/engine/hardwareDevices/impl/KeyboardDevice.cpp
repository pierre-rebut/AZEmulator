//
// Created by pierr on 03/08/2023.
//
#include "KeyboardDevice.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {
    KeyboardDevice::KeyboardDevice(const DeviceCreateData& deviceInfo) : HardwareDevice(deviceInfo.uuid, DeviceType::KEYBOARD, deviceInfo.name) {
        m_status = DeviceStatus::READY;
    }

    LARGE KeyboardDevice::Fetch(DataFormat, size_t) {
        std::scoped_lock guard(m_mtx);

        if (m_keyQueue.empty()) {
            return 0;
        }

        const auto& keyEntry = m_keyQueue.front();
        DWORD keyCode = (WORD) keyEntry.first | (keyEntry.second << 16);
        m_keyQueue.pop();

        return keyCode;
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{1, 0}};
    const std::vector<std::pair<size_t, size_t>>& KeyboardDevice::GetDeviceAddressList() const {
        return addrList;
    }

    void KeyboardDevice::SetControllerKey(KeyCode keyCode) {
        std::scoped_lock guard(m_mtx);
        m_keyQueue.emplace(keyCode, true);
    }

    void KeyboardDevice::UnsetControllerKey(KeyCode keyCode) {
        std::scoped_lock guard(m_mtx);
        m_keyQueue.emplace(keyCode, false);
    }


}
