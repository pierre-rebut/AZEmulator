//
// Created by pierr on 27/08/2023.
//

#include "MultiInputModal.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::Core {
    void MultiInputModal::OpenInput(std::string&& msg, OptionType&& options, std::function<bool(const OptionType &)>&& callback) {
        m_msg = std::move(msg);
        m_options = std::move(options);
        m_callback = std::move(callback);

        APopup::Open();
    }

    void MultiInputModal::drawPopupContent() {
        ImGui::Text(m_msg.c_str());

        for (auto& [optionName, optionVal] : m_options) {
            UI::Core::InputText(optionName, optionVal);
        }

        const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = ImGui::CalcTextSize(I18N::Get("CANCEL")).x + 36;

        UI::Core::ShiftCursorX(((contentRegionWidth - (buttonWidth * 2.0f)) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

        if (ImGui::Button(I18N::Get("OK"), ImVec2(buttonWidth, 0.0f))) {
            if (m_callback(m_options)) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button(I18N::Get("CANCEL"), ImVec2(buttonWidth, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }
    }
}