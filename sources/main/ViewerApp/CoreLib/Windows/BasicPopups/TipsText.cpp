//
// Created by pierr on 10/01/2024.
//
#include "TipsText.h"

#include "Commons/utils/YAMLimport.h"
#include "Commons/Log.h"
#include "ViewerApp/Custom/ViewerConstants.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Utils.h"

#include <filesystem>

namespace Astra::UI {
    TipsText::TipsText() {
        if (!std::filesystem::exists(UI::App::ViewerConstants::TIPS_FILE)) {
            return ;
        }

        try {
            YAML::Node data = YAML::LoadFile(UI::App::ViewerConstants::TIPS_FILE);

            m_isTipsEnable = data["TipsEnable"].as<bool>(true);

            for (const auto tip : data["Tips"]) {
                const auto name = tip.first.as<std::string>();
                const auto isVisible = tip.second.as<bool>(true);
                m_isTipsVisible[name] = isVisible;
            }
        } catch (const std::exception& e) {
            LOG_WARN("TipsText: {}", e.what());
        }
    }

    TipsText::~TipsText() {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "TipsEnable" << YAML::Value << m_isTipsEnable;
        out << YAML::Key << "Tips" << YAML::Value << YAML::BeginMap;

        for (const auto& [tipName, isTipVisible] : m_isTipsVisible) {
            out << YAML::Key << tipName << YAML::Value << isTipVisible;
        }

        out << YAML::EndMap;
        out << YAML::EndMap;

        std::ofstream file(UI::App::ViewerConstants::TIPS_FILE);
        if (file.is_open()) {
            file << out.c_str();
            file.close();
        }
    }

    void TipsText::Show(const char* tipId) {
        if (!m_isTipsEnable) {
            return;
        }

        if (!m_isTipsVisible.contains(tipId)) {
            m_isTipsVisible[tipId] = true;
        }

        if (m_isTipsVisible.at(tipId)) {
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetWindowContentRegionWidth() - 25);
            ImGui::TextDisabled(I18N::Get(tipId));
            ImGui::PopTextWrapPos();

            ImGui::SameLine();

            Core::ScopedStyle delStyle(ImGuiStyleVar_FramePadding, ImVec2(3,3));
            Core::ScopedColour btnColor(ImGuiCol_Button, Core::Colors::compliment);
            Core::ScopedColour btnHoverColor(ImGuiCol_ButtonHovered, Core::Colors::niceBlue);
            if (ImGui::Button(ICON_FA_EYE_SLASH)) {
                m_isTipsVisible[tipId] = false;
            }

        }
    }
}
