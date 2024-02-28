//
// Created by pierr on 16/03/2023.
//
#pragma once

#include <vector>

#include "Commons/utils/ObjectStack.h"
#include "Commons/utils/UUID.h"

#include "ViewerApp/CoreLib/System/TitleBar.h"
#include "ViewerApp/CoreLib/Graphics/AWindow.h"
#include "ViewerApp/CoreLib/Resources/Texture.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/CoreLib/System/NotificationSystem.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/MultiQuestionModal.h"
#include "ViewerApp/CoreLib/System/Events.h"

namespace Astra::UI::Core {

    class MainScreen
    {
    private:
        bool m_running = true;
        bool m_askExitPopup = false;

        NotificationSystem m_notificationSystem;
        Events m_eventsManager{m_notificationSystem};
        WindowsManager m_windowsManager;

        MultiQuestionModal m_questionModal{ICON_FA_PERSON_WALKING_DASHED_LINE_ARROW_RIGHT " Exit"};

    protected:
        Scope<TitleBar> m_titleBar;
        bool m_isInitialDisplay = true;

    public:
        MainScreen();

        virtual ~MainScreen();

        bool isRunning() const {return m_running;}

        virtual void init() = 0;
        virtual void displayInitScreen(const UI::Core::FrameInfo& pFrameInfo) = 0;
        virtual void setWindowFocusStartUp() const = 0;

        void OnUpdate(const UI::Core::FrameInfo& pFrameInfo);

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo);

        void ProcessEvents();

    private:
        virtual void defineCustomDockspace(ImGuiID dockMainId) const = 0;

        void renderMainScreen();

        void createDockSpace() const;
        void renderExitPopup();
        static void loadingScreen() ;
    };

}
