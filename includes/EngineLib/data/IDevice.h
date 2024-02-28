//
// Created by pierr on 15/08/2023.
//
#pragma once

#include <cstdint>
#include <string>

#include "EngineLib/data/Types.h"
#include "IComObject.h"

namespace Astra::CPU {

    class IDevice : public IComObject
    {
    public:
        const uint64_t deviceUUID;
        const std::string& deviceName;

        explicit IDevice(uint64_t uuid, const std::string& name) : deviceUUID(uuid), deviceName(name) {}
    };

}
