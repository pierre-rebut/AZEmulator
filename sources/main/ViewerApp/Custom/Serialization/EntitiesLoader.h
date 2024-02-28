//
// Created by pierr on 18/03/2023.
//
#pragma once

#include <filesystem>
#include <vector>

#include "EngineLib/data/Base.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"

namespace Astra::UI::App {

    class EntitiesLoader
    {
    public:
        EntitiesLoader() = delete;

        static bool loadConfig(const Ref<CPU::Core::CpuEngine>& engine,const std::filesystem::path& pFilename) noexcept;
        static bool saveValues(const Ref<CPU::Core::CpuEngine>& engine, const std::filesystem::path& pFilename) noexcept;
    };

}
