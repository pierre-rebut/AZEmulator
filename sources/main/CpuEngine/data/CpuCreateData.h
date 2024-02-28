//
// Created by pierr on 19/07/2023.
//

#pragma once

#include <string>
#include <vector>

#include "Commons/utils/UUID.h"

namespace Astra::CPU::Core {

    struct CpuCreateData
    {
        UUID uuid = 0;

        std::string name;
        bool autostart = true;
        size_t speed = 1;

        std::string coreLibDir;
        std::string coreLibName;

        int orderPriority = 1;

        std::vector<int> hardParameters;
    };
}
