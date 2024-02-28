//
// Created by pierr on 31/10/2023.
//
#include "SerialDevice.h"

#include <sstream>

namespace Astra::CPU::Core {
    SerialDevice::SerialDevice(const DeviceCreateData& deviceInfo) : HardwareDevice(deviceInfo.uuid, DeviceType::SERIAL, deviceInfo.name) {
        m_status = DeviceStatus::READY;
    }

    LARGE SerialDevice::Fetch(DataFormat fmt, size_t address) {
        std::scoped_lock guard(m_mtx);

        if (m_inMessage.empty()) {
            return 0;
        }

        LARGE c = m_inMessage.front();
        m_inMessage.pop();

        if (fmt != DataFormat::Byte && !m_inMessage.empty()) {
            c |= (m_inMessage.front() << 8);
            m_inMessage.pop();
        }

        return c;
    }

    void SerialDevice::Push(DataFormat fmt, size_t address, LARGE value) {
        std::scoped_lock guard(m_mtx);

        if (m_outMessage.size() > 1000) {
            return;
        }

        m_outMessage.emplace(value & 0xFF);
        if (fmt != DataFormat::Byte) {
            m_outMessage.emplace((value >> 8) & 0xFF);
        }

        if (value == '\n') {
            outMsg++;
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{2, 0}};

    const std::vector<std::pair<size_t, size_t>>& SerialDevice::GetDeviceAddressList() const {
        return addrList;
    }

    void SerialDevice::SendSerialMessage(const std::string& msg) {
        std::scoped_lock guard(m_mtx);

        if (m_inMessage.size() < 1000) {
            for (const auto c: msg) {
                m_inMessage.emplace(c);
            }
            m_inMessage.emplace('\n');
        }
    }

    void SerialDevice::UpdateList(std::list<std::pair<std::string, bool>>& msgList) {
        std::scoped_lock guard(m_mtx);
        std::stringstream ss;

        while (outMsg > 0) {
            auto c = m_outMessage.front();
            m_outMessage.pop();

            if (c == '\n') {
                msgList.emplace_back(ss.str(), false);
                ss = {};
                outMsg--;
            } else {
                ss << (char) c;
            }
        }
    }
}
