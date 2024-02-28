//
// Created by pierr on 25/08/2023.
//

#pragma once

#include <string>
#include <list>
#include <tuple>

#include "EngineLib/data/Base.h"
#include "Commons/utils/UUID.h"

#include "CpuEngine/engine/Device.h"

namespace Astra::CPU::Core {

    struct BusCreateData
    {
        UUID uuid;
        std::string name;
        size_t size;
        bool isReadOnly = false;

        std::list<std::tuple<UUID, size_t, int>> connectedDevices;
    };

} // Astra
