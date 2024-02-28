//
// Created by pierr on 16/03/2023.
//
#include "TestPanel.h"

#include "ViewerApp/CoreLib/Resources/Resources.h"

#include "imgui.h"
#include "ViewerApp/CoreLib/Project.h"

namespace Astra::UI::App {

    TestPanel::TestPanel() : APanel(NAME, UI::Core::PanelType::TOOLS) {
        m_isOpen = false;
    }

    void TestPanel::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) {
        if (m_isOpen && !UI::Core::Project::CurrentProject()->isIsFullScreen())
            ImGui::ShowDemoWindow(&m_isOpen);
    }
}
