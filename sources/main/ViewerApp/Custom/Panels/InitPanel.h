//
// Created by pierr on 28/03/2023.
//
#pragma once

#include <filesystem>
#include <list>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "ViewerApp/CoreLib/Colors.h"
#include "ViewerApp/Custom/Popups/NewProjectModal.h"

#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/Custom/Serialization/InitSerializer.h"

namespace Astra::UI::App {

    class InitPanel : public UI::Core::APanel
    {
    private:
        NewProjectModal* m_newProjectPanel = UI::Core::WindowsManager::Get().getPopups().get<NewProjectModal>();
        std::list<Ref<ProjectHistory>> m_projectsHistory;
        std::list<Ref<ProjectHistory>> m_searchProjectsHistory;

        std::string m_searchBuffer;

    public:
        static constexpr const char* NAME = "Welcome to AZ Emulator";

        InitPanel();

        ~InitPanel() override;

        void OnImGuiRender(const Core::FrameInfo& pFrameInfo) override;

        void OnEvent(UI::Core::AEvent& pEvent) override;

        const auto& GetProjectsHistory() const { return m_projectsHistory; }

    private:
        void drawPanelContent() override;

        void contentOutliner();

        void contentMain();

        static bool drawProjectHistoryItem(int i, const ProjectHistory& projectInfo);
        void addProjectToHistory(const std::string& projectName, const std::filesystem::path& projectPath);
        void drawTitlebar() const;
        void updateSearchList();
    };

}
