//
// Created by pierr on 28/07/2023.
//
#include "SimpleInputModal.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::Core {
    void SimpleInputModal::OpenInput(const std::string_view& msg, const std::string_view& defaultMsg,
                                     std::function<void(const char*)>&& callback, const Ref <UI::Core::Texture>& icon) {
        m_msg = msg;
        m_icon = icon;
        m_callback = callback;

        std::memset(m_buffer, 0, sizeof(m_buffer));
        std::strcpy(m_buffer, defaultMsg.data());

        APopup::Open();
    }

    void SimpleInputModal::drawPopupContent() {
        ImGui::Image(m_icon->textureId(), ImVec2(15, 15));

        ImGui::SameLine();

        ImGui::Text(m_msg.c_str());

        ImGui::SetItemDefaultFocus();
        bool res = ImGui::InputText("##val", m_buffer, sizeof(m_buffer),
                                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank);

        const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = ImGui::CalcTextSize(I18N::Get("CANCEL")).x + 36;

        UI::Core::ShiftCursorX(((contentRegionWidth - (buttonWidth * 2.0f)) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

        if (ImGui::Button(I18N::Get("OK"), ImVec2(buttonWidth, 0.0f)) || res) {
            if (strlen(m_buffer) > 0) {
                m_callback(m_buffer);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button(I18N::Get("CANCEL"), ImVec2(buttonWidth, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }
    }
}
