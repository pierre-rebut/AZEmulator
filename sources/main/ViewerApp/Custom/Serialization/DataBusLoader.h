//
// Created by pierr on 26/08/2023.
//

#pragma once

#include <filesystem>
#include <vector>

#include "CpuEngine/data/BusCreateData.h"

namespace Astra::UI::App {

    class DataBusLoader
    {
    public:
        DataBusLoader() = delete;

        static bool loadConfig(const std::filesystem::path& filename, std::vector<CPU::Core::BusCreateData>& data) noexcept;
        static bool saveConfig(const std::filesystem::path& filename) noexcept;
    };

} // Astra
