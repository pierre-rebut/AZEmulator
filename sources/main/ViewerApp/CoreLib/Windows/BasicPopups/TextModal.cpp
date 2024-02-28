//
// Created by pierr on 28/07/2023.
//
#include "TextModal.h"

#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::Core {
    void TextModal::OpenMessage(std::vector<std::string>&& msgs, const Ref<UI::Core::Texture>& icon) {
        m_msgs = std::move(msgs);
        m_icon = icon;
        APopup::Open();
    }

    void TextModal::drawPopupContent() {
        ImGui::Image(m_icon->textureId(), ImVec2(30,30));

        ImGui::SameLine();

        for (const auto& msg : m_msgs) {
            ImGui::Text(msg.c_str());
        }

        Core::PopupCloseButton();
    }
}
