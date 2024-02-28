//
// Created by pierr on 16/08/2023.
//

#include "CustomMainScreen.h"

#include "CustomTitleBar.h"

#include "ViewerApp/Custom/Panels/Views/ScreenPanel.h"
#include "ViewerApp/Custom/Panels/Views/EditorPanel.h"
#include "ViewerApp/Custom/Panels/Views/MemoryPanel.h"
#include "ViewerApp/Custom/Panels/Views/StatisticsPanel.h"
#include "ViewerApp/Custom/Panels/Views/ConfigPanel.h"
#include "ViewerApp/Custom/Panels/Views/EntitiesPanel.h"
#include "ViewerApp/Custom/Panels/Views/ExplorerPanel.h"
#include "ViewerApp/Custom/Panels/Views/LogPanel.h"
#include "ViewerApp/Custom/Panels/Views/NodesPanel.h"
#include "ViewerApp/Custom/Panels/Tools/TestPanel.h"
#include "ViewerApp/Custom/Panels/Tools/ToolsPanel.h"
#include "ViewerApp/Custom/Popups/MemoryConfigModal.h"
#include "ViewerApp/Custom/Popups/CoreLibModal.h"
#include "ViewerApp/Custom/Panels/Views/DebugPanel.h"
#include "ViewerApp/Custom/Panels/Views/SerialPanel.h"
#include "ViewerApp/Custom/Popups/ConfigDeviceModal.h"

namespace Astra::UI::App {

    void CustomMainScreen::init() {
        LOG_DEBUG("[CustomMainScreen] Init");

        initModals();
        initPanels();

        m_titleBar = CreateScope<CustomTitleBar>();

        LOG_DEBUG("[CustomMainScreen] Init END");
    }

    void CustomMainScreen::initPanels() {
        auto& panelsStack = UI::Core::WindowsManager::Get().getPanels();

        panelsStack.push<InitPanel>();
        panelsStack.push<MemoryPanel>();
        panelsStack.push<EntitiesPanel>();
        panelsStack.push<ExplorerPanel>();
        panelsStack.push<ScreenPanel>();
        panelsStack.push<LogPanel>();
        panelsStack.push<EditorPanel>();
        panelsStack.push<ConfigPanel>();
        panelsStack.push<StatisticsPanel>();
        panelsStack.push<NodesPanel>();
        panelsStack.push<DebugPanel>();
        panelsStack.push<SerialPanel>();

        panelsStack.push<TestPanel>();
        panelsStack.push<MetricToolPanel>();
        panelsStack.push<StackToolPanel>();
        panelsStack.push<DebugToolPanel>();
        panelsStack.push<StyleToolPanel>();
    }

    void CustomMainScreen::initModals() {
        auto& popupsStack = UI::Core::WindowsManager::Get().getPopups();

        popupsStack.push<ConfigEngineModal>();
        popupsStack.push<ConfigDeviceModal>();
        popupsStack.push<NewProjectModal>();
        popupsStack.push<SettingsModal>();
        popupsStack.push<MemoryConfigModal>();
        popupsStack.push<CoreLibModal>();

    }

    void CustomMainScreen::defineCustomDockspace(ImGuiID dockMainId) const {
        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.33f, nullptr, &dockMainId);
        ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.17f, nullptr, &dockMainId);
        ImGuiID dockDownId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.33f, nullptr, &dockMainId);

        ImGui::DockBuilderDockWindow(InitPanel::NAME, dockMainId);
        ImGui::DockBuilderDockWindow(ScreenPanel::NAME, dockMainId);
        ImGui::DockBuilderDockWindow(EditorPanel::NAME, dockMainId);
        ImGui::DockBuilderDockWindow(NodesPanel::NAME, dockMainId);
        ImGui::DockBuilderDockWindow(MemoryPanel::NAME, dockRightId);
        ImGui::DockBuilderDockWindow(StatisticsPanel::NAME, dockRightId);
        ImGui::DockBuilderDockWindow(ConfigPanel::NAME, dockRightId);
        ImGui::DockBuilderDockWindow(EntitiesPanel::NAME, dockLeftId);
        ImGui::DockBuilderDockWindow(ExplorerPanel::NAME, dockDownId);
        ImGui::DockBuilderDockWindow(LogPanel::NAME, dockDownId);
        ImGui::DockBuilderDockWindow(DebugPanel::NAME, dockDownId);
        ImGui::DockBuilderDockWindow(SerialPanel::NAME, dockDownId);

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockMainId);
        node->LocalFlags |= ImGuiDockNodeFlags_NoDockingSplitMe;
    }

    void CustomMainScreen::displayInitScreen(const UI::Core::FrameInfo& pFrameInfo) {
        auto initPanel = UI::Core::WindowsManager::Get().getPanels().get<InitPanel>();
        initPanel->OnImGuiRender(pFrameInfo);
    }

    void CustomMainScreen::setWindowFocusStartUp() const {
        ImGui::SetWindowFocus(ExplorerPanel::NAME);
        ImGui::SetWindowFocus(ConfigPanel::NAME);
        if (AstraProject::CurrentProject()->IsForceFocusEditor()) {
            ImGui::SetWindowFocus(EditorPanel::NAME);
        } else {
            ImGui::SetWindowFocus(ScreenPanel::NAME);
        }
    }

} // Astra