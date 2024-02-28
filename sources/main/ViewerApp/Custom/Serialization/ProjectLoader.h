//
// Created by pierr on 26/07/2023.
//
#pragma once

#include <filesystem>
#include "Commons/utils/UUID.h"

namespace Astra::UI::App {

    class AstraProject;

    class ProjectLoader
    {
    public:
        ProjectLoader() = delete;

        static bool loadConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept;
        static bool saveConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept;
    };
}
