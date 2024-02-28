//
// Created by pierr on 16/01/2024.
//
#pragma once

#include "CpuEngine/engine/Device.h"

#include <mutex>

namespace Astra::CPU::Core {

    class HardwareDevice : public Device
    {
    private:
        std::string m_name;

    protected:
        std::mutex m_mtx;
        DeviceStatus m_status = DeviceStatus::DISCONNECTED;

    public:
        HardwareDevice(UUID pUuid, DeviceType pType, std::string name) : Device(pUuid, pType, m_name), m_name(std::move(name)) {}

        DeviceStatus GetStatus() const { return m_status; }

        const std::string& GetName() const override { return m_name; }
        inline void SetName(const std::string_view& newName) { m_name = newName; }
    };

}
