//
// Created by pierr on 15/08/2023.
//
#pragma once

#include <filesystem>
#include <vector>
#include "Commons/utils/UUID.h"
#include "CpuEngine/data/DeviceCreateData.h"

namespace Astra::UI::App {

    class DeviceLoader
    {
    public:
        DeviceLoader() = delete;

        static bool loadConfig(const std::filesystem::path& pFilename, std::vector<CPU::Core::DeviceCreateData>& pData) noexcept;
        static bool saveConfig(const std::filesystem::path& pFilename) noexcept;
    };

}
