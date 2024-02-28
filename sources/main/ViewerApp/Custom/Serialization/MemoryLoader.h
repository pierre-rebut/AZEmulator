//
// Created by pierr on 26/08/2023.
//

#pragma once

#include <filesystem>

#include "EngineLib/data/Base.h"
#include "CpuEngine/manager/buses/DataBus.h"

namespace Astra::UI::App {

    class MemoryLoader
    {
    public:
        MemoryLoader() = delete;

        static bool loadAllMemories(const std::filesystem::path& projectDirectory);
        static bool saveAllMemories(const std::filesystem::path& projectDirectory);

        static bool loadMemory(const Ref<CPU::Core::DataBus>& dataBus, const std::filesystem::path& filepath, size_t startAddr = 0, size_t offset = 0) noexcept;
        static bool saveMemory(const Ref<CPU::Core::DataBus>& dataBus, const std::filesystem::path& filepath, size_t startAddr = 0) noexcept;
    };

} // Astra
