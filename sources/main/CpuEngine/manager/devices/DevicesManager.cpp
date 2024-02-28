//
// Created by pierr on 03/08/2023.
//

#include "DevicesManager.h"

#include "Commons/Log.h"
#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/engine/hardwareDevices/impl/KeyboardDevice.h"
#include "CpuEngine/manager/buses/DataBusManager.h"

#include "CpuEngine/engine/hardwareDevices/impl/ScreenDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/AudioDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/SerialDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/DiskDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/MouseDevice.h"

namespace Astra::CPU::Core {
    void DevicesManager::Init(const std::vector<DeviceCreateData>& devices) {
        LOG_CPU_DEBUG("[DevicesManager] Init {}", devices.size());

        for (const auto& deviceInfo: devices) {
            AddDevice(deviceInfo);
        }

        LOG_CPU_DEBUG("[DevicesManager] Init END");
    }

    void DevicesManager::Reset() {
        LOG_CPU_DEBUG("[DevicesManager] Reset");

        m_devices.clear();

        LOG_CPU_DEBUG("[DevicesManager] Reset END");
    }

    Ref<Device> DevicesManager::AddDevice(const DeviceCreateData& deviceInfo) {
        if (deviceInfo.type != DeviceType::ENGINE) {
            auto newDevice = createNewDevice(deviceInfo);
            if (!newDevice) {
                NOTIFY_WARN("Device manager", "Unknown device type {}", Device::convertDeviceTypeToString(deviceInfo.type));
                return nullptr;
            }

            m_devices[deviceInfo.uuid] = std::move(newDevice);
        } else {
            const auto& engineDevice = std::dynamic_pointer_cast<CpuEngine>(m_devices.at(deviceInfo.uuid));
            const auto& engineManager = EngineManager::Get();

            for (const auto& connectedCpu: deviceInfo.connectedCpu) {
                if (auto engineConnection = engineManager.GetEngineByUUID(connectedCpu)) {
                    engineDevice->ConnectCpuInterrupt(engineConnection);
                }
            }
        }

        return m_devices.at(deviceInfo.uuid);
    }

    Scope<Device> DevicesManager::createNewDevice(const DeviceCreateData& data) {
        LOG_CPU_DEBUG("[DevicesManager] Creating device type {} with id {}", Device::convertDeviceTypeToString(data.type), data.uuid);

        switch (data.type) {
            case DeviceType::KEYBOARD:
                return CreateScope<KeyboardDevice>(data);
            case DeviceType::MOUSE:
                return CreateScope<MouseDevice>(data);
            case DeviceType::SCREEN:
                return CreateScope<ScreenDevice>(data);
            case DeviceType::AUDIO:
                return CreateScope<AudioDevice>(data);
            case DeviceType::SERIAL:
                return CreateScope<SerialDevice>(data);
            case DeviceType::DISK:
                return CreateScope<DiskDevice>(data);
            default:
                return nullptr;
        }
    }

    void DevicesManager::addEngineDevice(const Ref<CpuEngine>& engineDevice) {
        LOG_CPU_DEBUG("[DevicesManager] addEngineDevice {}", engineDevice->deviceUUID);

        m_devices[engineDevice->deviceUUID] = engineDevice;

        LOG_CPU_DEBUG("[DevicesManager] addEngineDevice END");
    }

    void DevicesManager::RemoveDevice(UUID deviceUUID) {
        if (!m_devices.contains(deviceUUID)) {
            return;
        }

        LOG_CPU_DEBUG("[DevicesManager] RemoveDevice {}", deviceUUID);

        const Ref<Device>& device = m_devices.at(deviceUUID);
        DataBusManager::Get().RemoveDeviceLink(device);
        EngineManager::Get().UnbindDevice(device);

        m_devices.erase(deviceUUID);

        NOTIFY_INFO("Device manager", "Device {} removed", deviceUUID);
        LOG_CPU_DEBUG("[DevicesManager] RemoveDevice END");
    }
}
