//
// Created by pierr on 28/03/2023.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "InitPanel.h"

#include <fstream>

#include "imgui.h"
#include "imgui_internal.h"
#include "Commons/Profiling.h"
#include "Commons/utils/YAMLimport.h"

#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/CoreLib/CoreEngine.h"

#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/Custom/ViewerConstants.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "Commons/Log.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "Commons/utils/Utils.h"

namespace Astra::UI::App {
    InitPanel::InitPanel() : Core::APanel(NAME, Core::PanelType::NONE) {
        InitSerializer::load(m_projectsHistory);
        updateSearchList();
    }

    InitPanel::~InitPanel() {
        InitSerializer::update(m_projectsHistory);
    }

    void InitPanel::OnImGuiRender(const Core::FrameInfo& pFrameInfo) {
        ENGINE_PROFILE_FUNCTION();

        if (AstraProject::CurrentProject()) {
            return;
        }

        if (ImGui::Begin(NAME, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));
            ImGui::BeginChild("##InitWindow", ImVec2(1050, 500), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

            auto rect = ImGui::GetCurrentWindow()->Rect();
            ImGui::GetWindowDrawList()->AddRectFilled(rect.Min, rect.Max, IM_COL32(75, 21, 130, 100));

            drawTitlebar();
            drawPanelContent();

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void InitPanel::drawTitlebar() const {
        ImGui::SetCursorPosX(15);
        ImGui::Image(ViewerResources::LogoTexture->textureId(), ImVec2(25, 25));
        ImGui::SameLine(0, 20);
        ImGui::Text(NAME);

        const auto btnStr = myFormat(ICON_FA_PERSON_WALKING_DASHED_LINE_ARROW_RIGHT " {}", I18N::Get("EXIT"));
        ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(btnStr.c_str()).x + 80));

        if (UI::Core::Widgets::IconButton(ICON_FA_GEAR)) {
            ImGui::OpenPopup("LANGUAGE_SEL");
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 200, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 50, 230, 255));
        if (ImGui::Button(btnStr.c_str())) {
            UI::Core::Events::Get().OnEvent<UI::Core::WindowCloseEvent>();
        }
        ImGui::PopStyleColor(2);

        if (ImGui::BeginPopup("LANGUAGE_SEL")) {
            if (ImGui::BeginMenu(I18N::Get("LANGUAGE"))) {
                for (const auto& lang: I18NImpl::Get().TradList()) {
                    if (ImGui::MenuItem(lang.c_str(), nullptr, lang == I18NImpl::Get().Current())) {
                        I18NImpl::Get().LoadTrad(lang);
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }
    }

    void InitPanel::drawPanelContent() {
        UI::Core::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
        UI::Core::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

        Core::ScopedStyle style(ImGuiStyleVar_WindowPadding, ImVec2(20, 10));

        if (UI::Core::AsyncJob::Get().IsTaskWaiting()) {
            ImGui::Text(I18N::Get("INIT_TASK"));
        } else {
            ImGui::BeginChild("content_outliner", ImVec2(250, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
            contentOutliner();
            m_newProjectPanel->OnImGuiRender();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("content_main", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
            contentMain();
            ImGui::EndChild();
        }
    }

    void InitPanel::contentOutliner() {
        UI::Core::ScopedFont font(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::LARGE]);
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 200, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 50, 230, 255));

        const float buttonSize = 180;
        const float padding = ((ImGui::GetContentRegionAvail().y - (buttonSize * 2)) / 3);

        UI::Core::ShiftCursorY(padding);
        if (ImGui::Button(ICON_FA_PLUS, ImVec2(-1, buttonSize))) {
            m_newProjectPanel->Open();
        }

        UI::Core::ShiftCursorY(padding - 4);
        if (ImGui::Button(ICON_FA_FOLDER_OPEN, ImVec2(-1, buttonSize))) {
            const auto projectPath = UI::Core::FileManager::OpenFolder(I18N::Get("OPEN_PROJECT"), ".");
            if (!projectPath.empty() && AstraProject::LoadProject(projectPath, true)) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::PopStyleColor(2);

        // Draw side shadow
        ImRect windowRect = UI::Core::RectExpanded(ImGui::GetCurrentWindow()->Rect(), 0.0f, 0.0f);
        ImGui::PushClipRect(windowRect.Min, windowRect.Max, false);
        UI::Core::DrawShadowInner(ViewerResources::ShadowTexture, 20.0f, windowRect, 1.0f,
                                  windowRect.GetHeight() / 4.0f, false, true, false, false);
        ImGui::PopClipRect();
    }

    void InitPanel::contentMain() {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::SeparatorText(I18N::Get("PROJECT_HISTORY"));
        ImGui::PopStyleVar();

        ImGui::SetNextItemWidth(-30);
        if (UI::Core::Widgets::SearchWidget("searchBar", m_searchBuffer)) {
            updateSearchList();
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 10);
        if (UI::Core::Widgets::IconButton(ICON_FA_TRASH_CAN)) {
            m_searchBuffer.clear();
            m_searchProjectsHistory = m_projectsHistory;
        }

        if (m_searchProjectsHistory.empty()) {
            ImGui::Text(I18N::Get("NOTHING_TO_SHOW"));
            return;
        }

        ImGui::BeginChild("##projectList");

        int i = 0;
        for (const auto& projectInfo : m_searchProjectsHistory) {
            ImGui::PushID(i);
            bool res = drawProjectHistoryItem(i, *projectInfo);
            ImGui::PopID();
            i++;

            if (res) {
                auto exist = std::ranges::find_if(m_projectsHistory.begin(), m_projectsHistory.end(), [&projectInfo](const auto& project) {
                    return project == projectInfo;
                });

                if (exist != m_projectsHistory.end()) {
                    m_projectsHistory.erase(exist);
                }

                updateSearchList();
                break;
            }
        }

        ImGui::EndChild();
    }

    bool InitPanel::drawProjectHistoryItem(int i, const ProjectHistory& projectInfo) {
        const auto pos = ImGui::GetCursorPos();
        const auto iconPos = ImGui::GetCursorScreenPos();

        ImGui::SetNextItemAllowOverlap();
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 50, 180, 150));
        if (ImGui::Button("##btn", ImVec2(-14, 60)) &&
            AstraProject::LoadProject(projectInfo.path)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);

        bool res = false;

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)) {
            auto save = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionWidth() - 75, pos.y + 14));
            if (UI::Core::Widgets::IconButton(ICON_FA_TRASH_CAN)) {
                LOG_DEBUG("Erase project history {}", projectInfo.name);
                res = true;
            }
            ImGui::SetCursorPos(save);
        }

        static const std::array<ImU32, 6> colors = {
                IM_COL32(255, 150, 50, 100),
                IM_COL32(55, 250, 50, 100),
                IM_COL32(55, 150, 250, 100),
                IM_COL32(55, 150, 250, 100),
                IM_COL32(55, 250, 250, 100),
                IM_COL32(250, 150, 250, 100),
        };

        auto drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(iconPos + ImVec2(30, 15), iconPos + ImVec2(60, 45), colors.at(i % colors.size()), 6);
        drawList->AddText(iconPos + ImVec2(40, 18), IM_COL32(255, 255, 255, 255), Utils::ToUpper(myFormat("{}", projectInfo.name.at(0))).c_str());
        drawList->AddText(iconPos + ImVec2(80, 6), IM_COL32(255, 255, 255, 255), projectInfo.name.c_str());
        drawList->AddText(ImGui::GetIO().Fonts->Fonts[Core::Fonts::SMALL], 18, iconPos + ImVec2(80, 31), IM_COL32(153, 153, 153, 153), projectInfo.path.string().c_str());

        return res;
    }

    void InitPanel::OnEvent(UI::Core::AEvent& pEvent) {
        if (pEvent.GetEventType() == UI::Core::EventType::ProjectLoaded) {
            const auto& project = AstraProject::CurrentProject();
            addProjectToHistory(project->getProjectName(), project->rootDirectory);
        }
    }

    void InitPanel::addProjectToHistory(const std::string& projectName, const std::filesystem::path& projectPath) {
        auto exist = std::ranges::find_if(m_projectsHistory.begin(), m_projectsHistory.end(), [&projectName, &projectPath](const auto& project) {
            return project->name == projectName || project->path == projectPath;
        });

        if (exist != m_projectsHistory.end()) {
            m_projectsHistory.erase(exist);
        }

        m_projectsHistory.emplace_front(CreateRef<ProjectHistory>(projectName, projectPath));
    }

    void InitPanel::updateSearchList() {
        if (m_searchBuffer.empty()) {
            m_searchProjectsHistory = m_projectsHistory;
        } else {
            m_searchProjectsHistory.clear();

            for (const auto& projectInfo: m_projectsHistory) {
                if (Utils::StrContains(projectInfo->name, m_searchBuffer)) {
                    m_searchProjectsHistory.emplace_back(projectInfo);
                }
            }
        }
    }
}
