//
// Created by pierr on 02/08/2023.
//
#pragma once

#include <vector>

#include "EngineLib/data/IDevice.h"
#include "Commons/utils/UUID.h"

#include "EngineLib/data/Base.h"
#include "EngineLib/services/IInterruptService.h"

namespace Astra::CPU::Core {

    enum class DeviceType
    {
        UNKNOWN,
        ENGINE,
        KEYBOARD,
        SCREEN,
        AUDIO,
        SERIAL,
        DISK,
        MOUSE
    };

    enum class DeviceStatus
    {
        READY,
        DISCONNECTED,
        BUSY
    };

    static constexpr std::pair<DeviceType, const char*> DeviceTypes[] = {
            {DeviceType::ENGINE,   "Engine"},
            {DeviceType::KEYBOARD, "Keyboard"},
            {DeviceType::SCREEN,   "Screen"},
            {DeviceType::AUDIO,    "Audio"},
            {DeviceType::SERIAL,   "Serial"},
            {DeviceType::DISK,     "Disk"},
            {DeviceType::MOUSE,     "Mouse"},
    };

    class Device : public IDevice
    {
    public:
        const DeviceType type;

    public:
        Device(UUID pUuid, DeviceType pType, const std::string& name);

        virtual const std::string& GetName() const = 0;
        virtual const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const = 0;

        static std::string convertDeviceTypeToString(DeviceType type);
        static DeviceType convertDeviceTypeFromString(const std::string_view& type);
        static std::string convertDeviceStatusToString(DeviceStatus status);
    };

}
