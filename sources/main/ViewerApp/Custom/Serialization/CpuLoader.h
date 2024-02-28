//
// Created by pierr on 25/07/2023.
//
#pragma once

#include <vector>
#include <string>

#include "CpuEngine/data/CpuCreateData.h"
#include "ViewerApp/Custom/AstraProject.h"

namespace Astra::UI::App {
    class CpuLoader
    {
    public:
        CpuLoader() = delete;

        static bool loadConfig(const std::filesystem::path& pFilename, std::vector<CPU::Core::CpuCreateData>& pData, UUID& lastCurrentEngine) noexcept;
        static bool saveConfig(const std::filesystem::path& pFilename, const AstraProject* project) noexcept;
    };
}
