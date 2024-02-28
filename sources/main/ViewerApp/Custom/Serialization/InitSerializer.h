//
// Created by pierr on 16/01/2024.
//
#pragma once

#include <string>
#include <list>
#include <filesystem>

#include "EngineLib/data/Base.h"

namespace Astra::UI::App {

    struct ProjectHistory
    {
        std::string name;
        std::filesystem::path path;
    };

    class InitSerializer
    {
    public:
        InitSerializer() = delete;

        static void load(std::list<Ref<ProjectHistory>>& projectsHistory) noexcept;
        static void update(const std::list<Ref<ProjectHistory>>& projectsHistory) noexcept;
    };

}
