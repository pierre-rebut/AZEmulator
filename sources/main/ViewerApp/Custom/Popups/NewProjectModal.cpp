//
// Created by pierr on 20/06/2023.
//
#include "NewProjectModal.h"
#include "imgui.h"
#include "Commons/Profiling.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "Commons/format.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TipsText.h"
#include "ViewerApp/CoreLib/Resources/Resources.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6Brands.h"

namespace Astra::UI::App {

    const ProjectTemplate NewProjectModal::TEMPLATE_LIST[] = {
            {nullptr, ICON_FA_TROWEL_BRICKS,       "EMPTY_PROJECT", "EMPTY_PROJECT_DESC"},
            {"65c02", ICON_FA_BREAD_SLICE, "SIMPLE_6502",   "SIMPLE_6502_DESC"},
            {"i8086", ICON_FA_BRAIN,       "SIMPLE_8086",   "SIMPLE_8086_DESC"},
            {"msbasic",   ICON_FA_MICROSOFT,     "MSBASIC",  "MSBASIC_DESC"},
            {"nes",   ICON_FA_GAMEPAD,     "NES_EMULATOR",  "NES_EMULATOR_DESC"},
            {"x16",   ICON_FA_MIXER,     "X16_EMULATOR",  "X16_EMULATOR_DESC"},
    };

    void NewProjectModal::drawPopupContent() {
        UI::Core::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
        UI::Core::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

        Core::ScopedStyle style(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

        ImGui::BeginChild("content_outliner", ImVec2(250, 450), ImGuiWindowFlags_AlwaysUseWindowPadding);
        contentOutliner();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("content_main", ImVec2(800, 450), ImGuiWindowFlags_AlwaysUseWindowPadding);
        contentNewProject();
        ImGui::EndChild();
    }

    void NewProjectModal::contentOutliner() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::SeparatorText("Template");
        ImGui::PopStyleVar();
        ImGui::Spacing();

        Core::ScopedStyle style(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

        for (const auto& templateItem: TEMPLATE_LIST) {

            if (&templateItem == m_template) {
                ImGui::PushStyleColor(ImGuiCol_Button, Core::Colors::compliment);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Core::Colors::compliment);
            }

            if (ImGui::Button(myFormat(" {} {}", templateItem.icon, I18N::Get(templateItem.name)).c_str(), ImVec2(-1, 0))) {
                m_template = &templateItem;
            }

            if (&templateItem == m_template) {
                ImGui::PopStyleColor(2);
            }
        }

        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 50);
        {
            UI::Core::ScopedFont font(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::LARGE]);
            ImGui::Text(ICON_FA_CIRCLE_QUESTION);
        }
        Core::SetTooltip(I18N::Get("TEMPLATE_INFO"), -1);

        // Draw side shadow
        ImRect windowRect = UI::Core::RectExpanded(ImGui::GetCurrentWindow()->Rect(), 0.0f, 10.0f);
        ImGui::PushClipRect(windowRect.Min, windowRect.Max, false);
        UI::Core::DrawShadowInner(ViewerResources::ShadowTexture, 20.0f, windowRect, 1.0f,
                                  windowRect.GetHeight() / 4.0f, false, true, false, false);
        ImGui::PopClipRect();

    }

    void NewProjectModal::contentNewProject() {
        ENGINE_PROFILE_FUNCTION();

        UI::Core::ScopedStyle style(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(190, 190, 190, 150));
        ImGui::PushTextWrapPos();
        ImGui::Text(I18N::Get(m_template->description));
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushItemWidth(-180);

        UI::Core::InputText(I18N::Get("PROJECT_NAME"), m_projectName);

        ImGui::Spacing();

        UI::Core::InputText(I18N::Get("LOCATION"), m_projectTmpPath);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN)) {
            m_projectTmpPath = UI::Core::FileManager::OpenFolder(I18N::Get("SELECT_DIRECTORY"), ".");
        }

        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(190, 190, 190, 150));

        m_projectPath = std::filesystem::path(m_projectTmpPath) / m_projectName;

        ImGui::Text(myFormat(I18N::Get("PROJECT_CREATED_AT"), m_projectPath.string() ).c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        bool tipsEnable = TipsText::Get().IsEnable();
        if (ImGui::Checkbox(I18N::Get("IS_TIPS_ENABLE"), &tipsEnable)) {
            TipsText::Get().SetEnable(tipsEnable);
            LOG_DEBUG("Tips enable: {}", tipsEnable);
        }

        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 40);

        const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = 110.0f;

        UI::Core::ShiftCursorX(((contentRegionWidth - (buttonWidth * 2.0f)) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 200, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 50, 230, 255));
        if (ImGui::Button(myFormat(ICON_FA_PLUS " {}", I18N::Get("CREATE")).c_str(), ImVec2(buttonWidth, 40)) && AstraProject::CreateProject(m_projectPath, m_projectName, m_template->id)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0, 30);

        if (ImGui::Button(myFormat(ICON_FA_ROTATE_LEFT " {}", I18N::Get("CANCEL")).c_str(), ImVec2(buttonWidth, 40))) {
            ImGui::CloseCurrentPopup();
        }
    }
}
