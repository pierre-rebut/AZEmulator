//
// Created by pierr on 26/06/2023.
//
#include "APopup.h"

#include "imgui.h"
#include "Commons/format.h"
#include "Commons/Profiling.h"

namespace Astra::UI::Core {
    APopup::APopup(std::string pName) : Name(std::move(pName)) {
    }

    void APopup::Open() {
        m_isOpen = true;
        ImGui::OpenPopup(Name.c_str());
    }

    std::string APopup::toString() const {
        return myFormat("{}", Name);
    }

    void APopup::OnImGuiRender() {
        ENGINE_PROFILE_FUNCTION();
        if (ImGui::BeginPopup(Name.c_str(), ImGuiWindowFlags_AlwaysAutoResize)) {
            if (!m_isOpen) {
                ImGui::CloseCurrentPopup();
            }

            drawPopupContent();

            ImGui::EndPopup();
        }
    }

    void AModal::OnImGuiRender() {
        ENGINE_PROFILE_FUNCTION();
        if (ImGui::BeginPopupModal(Name.c_str(), &m_isOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysUseWindowPadding)) {
            if (!m_isOpen) {
                ImGui::CloseCurrentPopup();
            }

            drawPopupContent();

            ImGui::EndPopup();
        }
    }
}
