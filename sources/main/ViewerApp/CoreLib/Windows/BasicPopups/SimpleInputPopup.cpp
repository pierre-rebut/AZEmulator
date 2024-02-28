//
// Created by pierr on 29/07/2023.
//

#include <cstring>
#include "SimpleInputPopup.h"
#include "imgui.h"

namespace Astra::UI::Core {
    void SimpleInputPopup::OpenMessage(int category, const std::string_view& defaultMsg) {
        m_category = category;
        m_isValid = false;
        std::memset(m_buffer, 0, sizeof(m_buffer));
        std::strcpy(m_buffer, defaultMsg.data());

        APopup::Open();
    }

    void SimpleInputPopup::drawPopupContent() {
        bool res = ImGui::InputText("##val", m_buffer, sizeof(m_buffer),
                                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank);

        if (ImGui::Button("OK") || res) {
            if (strlen(m_buffer) > 0) {
                m_isValid = true;
                ImGui::CloseCurrentPopup();
            }
        }
    }

    const char* SimpleInputPopup::getValue() {
        if (!m_isValid) {
            return nullptr;
        }

        m_isValid = false;
        return m_buffer;
    }
}
