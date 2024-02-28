//
// Created by pierr on 29/07/2023.
//
#pragma once

#include <filesystem>

namespace Astra::UI::App {

    class AstraProject;

    class LogLoader
    {
    public:
        LogLoader() = delete;

        static bool loadConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept;
        static bool saveConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept;
    };

}
