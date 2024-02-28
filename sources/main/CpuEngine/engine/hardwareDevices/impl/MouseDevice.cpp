//
// Created by pierr on 06/02/2024.
//
#include "MouseDevice.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {
    MouseDevice::MouseDevice(const DeviceCreateData& deviceInfo) : HardwareDevice(deviceInfo.uuid, DeviceType::MOUSE, deviceInfo.name) {
        m_status = DeviceStatus::READY;
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{1, 0}};

    const std::vector<std::pair<size_t, size_t>>& MouseDevice::GetDeviceAddressList() const {
        return addrList;
    }

    LARGE MouseDevice::Fetch(DataFormat, size_t) {
        std::scoped_lock guard(m_mtx);

        if (m_mouseQueue.empty()) {
            return 0;
        }

        auto ret = m_mouseQueue.front();
        m_mouseQueue.pop();
        return ret;
    }

    void MouseDevice::SetButtonDown(MouseCode code) {
        buttons |= 1 << (int) code;
        isMouseUpdated = true;
    }

    void MouseDevice::SetButtonUp(MouseCode code) {
        buttons &= (1 << (int) code) ^ 0xff;
        isMouseUpdated = true;
    }

    void MouseDevice::SetWheel(float y) {
        if (y < -7) {
            wheel = 7;
        } else if (y > 8) {
            wheel = -8;
        } else {
            wheel = -y;
        }

        isMouseUpdated = true;
    }

    void MouseDevice::SetMove(float x, float y) {

        mouseDiffX = (x - prevMouseX);
        mouseDiffY = - (y - prevMouseY);

        prevMouseX = x;
        prevMouseY = y;

        isMouseUpdated = true;
    }

    void MouseDevice::Update() {
        if (!isMouseUpdated) {
            return;
        }

        {
            std::scoped_lock guard(m_mtx);

            if (m_mouseQueue.size() >= MAX_BUFFER_SIZE) {
                return;
            }

            LARGE value = buttons;
            value |= ((LARGE) ((WORD) mouseDiffX)) << 8;
            value |= ((LARGE) ((WORD) mouseDiffY)) << 24;
            value |= ((LARGE) ((BYTE) wheel)) << 40;
            value |= (LARGE) 1 << 63;

            m_mouseQueue.emplace(value);
        }

        wheel = 0;
        mouseDiffX = 0;
        mouseDiffY = 0;

        isMouseUpdated = false;
    }
}
