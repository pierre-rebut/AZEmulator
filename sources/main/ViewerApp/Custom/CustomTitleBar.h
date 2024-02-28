//
// Created by pierr on 16/08/2023.
//

#pragma once

#include "ViewerApp/CoreLib/System/TitleBar.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/Custom/Popups/SettingsModal.h"
#include "ViewerApp/Custom/Popups/NewProjectModal.h"
#include "ViewerApp/Custom/Panels/InitPanel.h"
#include "ViewerApp/Custom/Popups/CoreLibModal.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TextModal.h"

namespace Astra::UI::App {
    class CustomTitleBar : public UI::Core::TitleBar
    {
    private:
        InitPanel* m_initPanel = UI::Core::WindowsManager::Get().getPanels().get<InitPanel>();
        NewProjectModal* m_newProjectPanel = UI::Core::WindowsManager::Get().getPopups().get<NewProjectModal>();
        CoreLibModal* m_coreLibModal = UI::Core::WindowsManager::Get().getPopups().get<CoreLibModal>();
        SettingsModal* m_settingsModal = UI::Core::WindowsManager::Get().getPopups().get<SettingsModal>();

        Core::TextModal m_aboutModal{ICON_FA_ADDRESS_CARD " About"};

        ObjectStack<UI::Core::APanel>& m_panelsStack = UI::Core::WindowsManager::Get().getPanels();

    private:
        void drawMenuItems(bool& menuOpen) override;
        void drawCustomStatus() override;
        void drawProjectNameInfo() override;

        void drawMenuFile(bool& menuOpen);

        static void drawMenuEdit(bool& menuOpen) ;

        void drawMenuView(bool& menuOpen) const;

        void drawMenuTools(bool& menuOpen) const;

        void drawMenuHelp(bool& menuOpen) ;

        static std::tuple<const char*, int, bool> getStatusValue();
        static void drawTooltipsOnNoProject() ;
    };
}