//
// Created by pierr on 26/03/2023.
//
#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <list>

#include "EngineLib/data/Base.h"
#include "Commons/utils/UUID.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "ViewerApp/CoreLib/Project.h"

namespace Astra::UI::App {

    struct ProjectSettings
    {
        int ContentBrowserThumbnailSize = 128;
        bool ContentBrowserShowAssetTypes = true;

        bool DisplaySecondaryEntities = false;

        int LogLevel = 0;
        bool LogAutoCleanOnRun = false;
        int CpuSpeed = 500;

        uint64_t screenUUID = 0;
        uint64_t keyboardUUID = 0;
        uint64_t mouseUUID = 0;
        uint64_t audioUUID = 0;
        uint64_t serialUUID = 0;
        bool autoSaving = true;
    };

    struct ProjectData
    {
        struct
        {
            int nodeCurrentIndex = 1;
            std::unordered_map<UUID, int> nodeId;
        } nodePanel;

        std::vector<UUID> openedEditorFiles;
        std::list<Ref<AstraMessage>> logMessages;
        std::list<std::pair<std::string, bool>> serialMessages;
    };

    class AstraProject : public UI::Core::Project
    {
    private:
        ProjectSettings m_settings;
        ProjectData m_data;

        Ref<CPU::Core::CpuEngine> m_currentCpuEngine;

        bool m_forceFocusEditor = false;

    public:
        inline static AstraProject* CurrentProject() {return dynamic_cast<AstraProject*>(UI::Core::Project::CurrentProject());}

        AstraProject(std::string projectName, std::filesystem::path mainDirectory);
        ~AstraProject() override;

        void SaveProject() noexcept override;

        inline ProjectSettings& getSettings() {
            return m_settings;
        }

        inline ProjectData& GetData() {
            return m_data;
        }

        inline Ref<CPU::Core::CpuEngine> getCurrentEngine() const {
            return m_currentCpuEngine;
        }

        void setCurrentEngine(const Ref<CPU::Core::CpuEngine>& newEngine) {
            m_currentCpuEngine = newEngine;
        }

        bool IsForceFocusEditor() const {return m_forceFocusEditor;}

        static bool LoadProject(const std::filesystem::path& pProjectPath, bool pCreateOnFailed = false) noexcept;
        static bool CreateProject(const std::filesystem::path& pProjectPath, const std::string& pProjectName, const char* templateId) noexcept;

    private:
        void initProject(bool openReadme) noexcept;
        void saveProjectInternal() noexcept;

        static void createEmptyProject(const std::string& pProjectName, const std::filesystem::path& astraPath);
        static void loadProjectInternal(const std::filesystem::path& pProjectPath, bool openReadme) noexcept;

        static void createProjectFromTemplate(const std::filesystem::path& path, const char* id);
    };

}
