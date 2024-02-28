//
// Created by pierr on 05/03/2022.
//

#include "APanel.h"

#include "imgui.h"

#include "Commons/format.h"
#include "Commons/Profiling.h"
#include "ViewerApp/CoreLib/Project.h"

namespace Astra::UI::Core {
    APanel::APanel(std::string pName, PanelType pType) : Name{std::move(pName)}, Type{pType} {
    }

    std::string APanel::toString() const {
        return myFormat("{}<{}>", Name, m_isOpen ? "open" : "close");
    }

    void APanel::Open() {
        m_isOpen = true;
        ImGui::SetWindowFocus(Name.c_str());
    }

    void APanel::OnImGuiRender(const FrameInfo&) {
        if (!m_isOpen || Project::CurrentProject()->isIsFullScreen()) { return; }

        ENGINE_PROFILE_FUNCTION();
        if (ImGui::Begin(Name.c_str(), &m_isOpen, ImGuiWindowFlags_NoCollapse)) {
            drawPanelContent();
        }

        ImGui::End();
    }
}
