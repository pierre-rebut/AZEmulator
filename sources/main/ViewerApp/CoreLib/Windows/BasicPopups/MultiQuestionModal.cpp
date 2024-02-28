//
// Created by pierr on 23/08/2023.
//

#include "MultiQuestionModal.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"

namespace Astra::UI::Core {
    void MultiQuestionModal::OpenQuestion(std::string&& question, std::vector<std::string>&& options, std::function<void(int)>&& callback) {
        m_msg = std::move(question);
        m_options = std::move(options);
        m_callback = std::move(callback);

        APopup::Open();
    }

    void MultiQuestionModal::drawPopupContent() {
        ImGui::Text(m_msg.c_str());

        const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = ImGui::CalcTextSize(std::ranges::max_element(m_options.begin(), m_options.end(),
                                                                               [](const auto& a, const auto& b) { return a.size() < b.size(); }
        )->c_str()).x + 36;

        UI::Core::ShiftCursorX(((contentRegionWidth - (buttonWidth * m_options.size())) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

        int opt = 0;
        for (const auto& option: m_options) {
            if (opt != 0) {
                ImGui::SameLine();
            }

            if (ImGui::Button(option.c_str(), ImVec2(buttonWidth, 0.0f))) {
                m_callback(opt);
                ImGui::CloseCurrentPopup();
            }

            opt++;
        }
    }
} // Astra