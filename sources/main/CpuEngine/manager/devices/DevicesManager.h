//
// Created by pierr on 03/08/2023.
//
#pragma once

#include <unordered_map>

#include "Commons/utils/UUID.h"
#include "EngineLib/data/Base.h"

#include "CpuEngine/data/DeviceCreateData.h"
#include "CpuEngine/engine/Device.h"
#include "CpuEngine/engine/hardwareDevices/RamMemory.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"

namespace Astra::CPU::Core {

    using DevicesList = std::unordered_map<UUID, Ref<Device>>;

    class DevicesManager : public Singleton<DevicesManager>
    {
    private:
        DevicesList m_devices;

        static Scope<Device> createNewDevice(const DeviceCreateData& data);

    public:
        static constexpr const char* NAME = "DevicesManager";

        void Init(const std::vector<DeviceCreateData>& devices);
        void Reset();

        Ref<Device> AddDevice(const DeviceCreateData& deviceInfo);
        void addEngineDevice(const Ref<CpuEngine>& engineDevice);
        void RemoveDevice(UUID deviceUUID);

        const DevicesList& GetDevices() const { return m_devices; }

        template<class T = Device>
        Ref<T> GetDeviceOfType(UUID deviceUUID) const {
            if (!m_devices.contains(deviceUUID)) {
                return nullptr;
            }

            return std::dynamic_pointer_cast<T>(m_devices.at(deviceUUID));
        }

        Ref<Device> GetDeviceUUID(UUID deviceUUID) const {
            if (!m_devices.contains(deviceUUID)) {
                return nullptr;
            }

            return m_devices.at(deviceUUID);
        }
    };

}
