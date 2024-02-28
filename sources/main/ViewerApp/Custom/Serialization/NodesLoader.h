//
// Created by pierr on 05/09/2023.
//

#pragma once

#include <filesystem>
#include "ViewerApp/Custom/AstraProject.h"

namespace Astra::UI::App {

    class NodesLoader
    {
    public:
        NodesLoader() = delete;

        static bool loadConfig(const std::filesystem::path& pFilename, ProjectData& projectData) noexcept;
        static bool saveConfig(const std::filesystem::path& pFilename, const ProjectData& projectData) noexcept;
    };

} // Astra
