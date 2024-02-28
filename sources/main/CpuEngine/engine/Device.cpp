//
// Created by pierr on 02/08/2023.
//
#include "Device.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {
    Device::Device(UUID pUuid, DeviceType pType, const std::string& name) : IDevice(pUuid, name), type(pType) {
    }

    std::string Device::convertDeviceTypeToString(DeviceType type) {
        for (const auto& [deviceType, deviceTypeName]: DeviceTypes) {
            if (deviceType == type) {
                return deviceTypeName;
            }
        }

        return "unknown";
    }

    DeviceType Device::convertDeviceTypeFromString(const std::string_view& type) {
        for (const auto& [deviceType, deviceTypeName]: DeviceTypes) {
            if (deviceTypeName == type) {
                return deviceType;
            }
        }

        return DeviceType::UNKNOWN;
    }

    std::string Device::convertDeviceStatusToString(DeviceStatus status) {
        switch (status) {
            case DeviceStatus::READY:
                return "READY";
            case DeviceStatus::BUSY:
                return "BUSY";
            default:
                return "DISCONNECTED";
        }
    }
}
