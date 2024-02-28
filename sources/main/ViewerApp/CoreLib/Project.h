//
// Created by pierr on 17/08/2023.
//

#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <atomic>

#include "EngineLib/data/Base.h"

namespace Astra::UI::Core {

    class Project
    {
    public:
        static constexpr const char* NAME = "Project";

    private:
        inline static Ref<Project> s_currentProject = nullptr;

    public:
        const std::filesystem::path rootDirectory;

    private:
        std::string m_projectName;
        bool m_isFullScreen = false;

    protected:
        std::atomic_bool m_isProjectLoaded = false;

    public:
        inline static void SetCurrentProject(const Ref<Project>& newProject) { s_currentProject = newProject; }

        inline static Project* CurrentProject() { return s_currentProject.get(); }

        virtual ~Project();

        virtual void SaveProject() noexcept = 0;

        inline const std::string& getProjectName() const { return m_projectName; }
        void setProjectName(const std::string_view& newName) { m_projectName = newName; }

        inline bool isIsFullScreen() const { return m_isFullScreen;}
        void setIsFullScreen(bool isFullScreen) {m_isFullScreen = isFullScreen;}

        inline bool IsProjectLoaded() const {return m_isProjectLoaded;}

    protected:
        Project(std::string pProjectName, std::filesystem::path pRootDirectory);
    };

} // Astra
