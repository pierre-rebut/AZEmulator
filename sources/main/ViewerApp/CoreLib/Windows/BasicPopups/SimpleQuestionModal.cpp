//
// Created by pierr on 10/08/2023.
//
#include "SimpleQuestionModal.h"

#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::Core {
    void SimpleQuestionModal::OpenQuestion(std::string&& question, std::function<void(bool)>&& callback) {
        m_msg = question;
        m_callback = callback;

        APopup::Open();
    }

    void SimpleQuestionModal::drawPopupContent() {

        ImGui::Text(m_msg.c_str());

        const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = ImGui::CalcTextSize(I18N::Get("YES")).x + 36;

        UI::Core::ShiftCursorX(((contentRegionWidth - (buttonWidth * 2.0f)) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

        if (ImGui::Button(I18N::Get("YES"), ImVec2(buttonWidth, 0.0f))) {
            m_callback(true);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button(I18N::Get("NO"), ImVec2(buttonWidth, 0.0f))) {
            m_callback(false);
            ImGui::CloseCurrentPopup();
        }
    }
}
