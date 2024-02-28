//
// Created by pierr on 29/10/2023.
//
#pragma once

#include <vector>

namespace Astra::CPU {

    struct CoreConfigInfo
    {
        bool allowDeviceConnection = false;
        std::vector<const char*> hardParameters{};
        std::vector<const char*> debugHeader{};
    };
}
