//
// Created by pierr on 16/03/2023.
//
#include "MainScreen.h"

#include "Commons/Profiling.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "imspinner.h"

namespace Astra::UI::Core {
    MainScreen::MainScreen() {
        Events::SetSingleton(&m_eventsManager);
        WindowsManager::SetSingleton(&m_windowsManager);
    }

    MainScreen::~MainScreen() {
        Project::SetCurrentProject(nullptr);
        Events::SetSingleton(nullptr);
        WindowsManager::SetSingleton(nullptr);
    }

    void MainScreen::ProcessEvents() {
        ENGINE_PROFILE_FUNCTION();

        while (auto event = m_eventsManager.pullEvents()) {
            switch (event->GetEventType()) {
                case EventType::DelayedAction: {
                    const auto actionEvt = dynamic_cast<const DelayedActionEvent*>(event.get());
                    actionEvt->execute();
                    break;
                }
                case EventType::WindowClose: {
                    if (!Project::CurrentProject()) {
                        m_running = false;
                    } else {
                        m_askExitPopup = true;
                    }
                    break;
                }
                case EventType::ProjectLoaded:
                    m_isInitialDisplay = true;
                default: {
                    LOG_TRACE("ProcessEvents: {}", event->toString());
                    if (Project::CurrentProject() && Project::CurrentProject()->IsProjectLoaded()) {
                        m_windowsManager.OnEvent(*event);
                    }
                }
            }
        }
    }

    void MainScreen::OnUpdate(const UI::Core::FrameInfo& pFrameInfo) {
        ENGINE_PROFILE_FUNCTION();

        if (Project::CurrentProject() && Project::CurrentProject()->IsProjectLoaded()) {
            m_windowsManager.OnUpdate(pFrameInfo);
        }
    }

    void MainScreen::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) {
        ENGINE_PROFILE_FUNCTION();

        renderMainScreen();
        renderExitPopup();

        if (auto project = Project::CurrentProject()) {
            if (project->IsProjectLoaded()) {
                m_windowsManager.OnImGuiRender(pFrameInfo);

                if (m_isInitialDisplay) {
                    setWindowFocusStartUp();
                    m_isInitialDisplay = false;
                }
            } else {
                loadingScreen();
            }
        } else {
            displayInitScreen(pFrameInfo);
        }

        m_notificationSystem.OnImGuiRender();
    }

    void MainScreen::loadingScreen() {
        ImGui::OpenPopup("Loading");

        if (ImGui::BeginPopupModal("Loading", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
            ImSpinner::SpinnerAng("##loading", 12, 6, ImSpinner::white, ImColor(255, 255, 255, 128), 8.f, ImSpinner::PI_DIV_2);
            ImGui::SameLine();
            ImGui::Text(I18N::Get("LOADING"));
            ImGui::EndPopup();
        }
    }

    void MainScreen::renderMainScreen() {
        ENGINE_PROFILE_FUNCTION();

        ImGuiIO& io = ImGui::GetIO();
        const AWindow& m_window = CoreEngine::Get().GetWindow();

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::FocusWindow(GImGui->HoveredWindow);
        }

        io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        bool isMaximized = m_window.isWindowMaximized();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

        ImGui::Begin("DockSpace Window", nullptr, window_flags);

        ImGui::PopStyleColor(); // MenuBarBg
        ImGui::PopStyleVar(2);
        ImGui::PopStyleVar(2);

        // Draw window border if the window is not maximized
        if (!isMaximized) {
            const UI::Core::ScopedColour windowBorder(ImGuiCol_Border, IM_COL32(50, 50, 50, 255));
            UI::Core::RenderWindowOuterBorders(ImGui::GetCurrentWindow());
        }

        ImVec2 newSize;
        ImVec2 newPosition;

        if (!isMaximized && UI::Core::UpdateWindowManualResize(ImGui::GetCurrentWindow(), newSize, newPosition)) {
            m_window.setWindowPos(newPosition.x, newPosition.y);
            m_window.setWindowSize(newSize.x, newSize.y);
        }

        m_titleBar->DrawTitleBar();

        ImGui::SetCursorPosY(TitleBar::titleBarHeight + ImGui::GetCurrentWindow()->WindowPadding.y);
        createDockSpace();

        ImGui::End();
    }

    void MainScreen::createDockSpace() const {
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 250;

        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
        ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
        if (!ImGui::DockBuilderGetNode(dockspaceID)) {
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);

            ImGuiID dockMainId = dockspaceID;

            defineCustomDockspace(dockMainId);

            ImGui::DockBuilderFinish(dockMainId);
        }

        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
        style.WindowMinSize.x = minWinSizeX;
    }

    void MainScreen::renderExitPopup() {
        if (m_askExitPopup) {
            m_askExitPopup = false;
            m_questionModal.OpenQuestion(I18N::Get("SAVE_EXIT"), {I18N::Get("YES"), I18N::Get("NO"), I18N::Get("CANCEL")}, [this](int res) {
                if (res == 0) {
                    Project::CurrentProject()->SaveProject();
                }

                if (res != 2) {
                    m_running = false;
                }
            });
        }

        m_questionModal.OnImGuiRender();
    }

}
