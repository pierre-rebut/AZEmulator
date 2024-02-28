//
// Created by pierr on 24/03/2023.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include <chrono>
#include <format>

#include "LogPanel.h"

#include "Commons/Profiling.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/AstraProject.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/Events/NotificationEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {
    LogPanel::LogPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

    void LogPanel::OnUpdate(const UI::Core::FrameInfo& pFrameInfo) {
        std::list<Ref<AstraMessage>> newMsgs;
        m_cpuLog.RetrieveMessages(newMsgs);

        for (const auto& msg: newMsgs) {
            //UI::Core::NotificationSystem::GetLink().Notify(UI::Core::NotificationToast(msg));
            m_logs->emplace_front(msg);
        }
    }

    void LogPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        if (!m_logs) {
            return;
        }

        TipsText::Get().Show("LOG_PANEL_INFO");

        showMenuBar();

        ImGui::Separator();

        showTableLogs();
    }

    void LogPanel::showMenuBar() {
        ImGui::Text(myFormat(ICON_FA_TIMELINE " {}", I18N::Get("TIMELINE")).c_str());

        ImGui::SameLine();
        const auto btnStr = myFormat(ICON_FA_BROOM " {}", I18N::Get("CLEAR"));
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(btnStr.c_str()).x + 60));

        if (ImGui::Button(btnStr.c_str())) {
            m_logs->clear();
        }

        ImGui::SameLine();

        ProjectSettings& settings = AstraProject::CurrentProject()->getSettings();
        if (UI::Core::Widgets::IconButton(ICON_FA_GEAR)) {
            ImGui::OpenPopup("Options");
        }
        UI::Core::SetTooltip(I18N::Get("LOG_SETTINGS"));

        if (UI::Core::BeginPopup("Options")) {
            Core::Toggle(I18N::Get("AUTO_CLEAN"), settings.LogAutoCleanOnRun);
            UI::Core::EndPopup();
        }
    }

    void LogPanel::showTableLogs() {

        UI::Core::ScopedStyle style(ImGuiStyleVar_WindowPadding, ImVec2(15, 10));
        UI::Core::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::MONO_BOLD]);

        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
        auto winSizeX = ImGui::GetWindowContentRegionWidth();
        auto drawList = ImGui::GetWindowDrawList();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 15));

        const auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto nowLocal = std::localtime(&nowTime);
        nowLocal->tm_hour = 0;
        nowLocal->tm_min = 0;
        nowLocal->tm_sec = 0;
        const auto midnightTimeT = std::mktime(nowLocal);
        const auto midnightTime = std::chrono::system_clock::from_time_t(midnightTimeT);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,3));

        ImGuiListClipper clipper;
        clipper.Begin((int) m_logs->size());
        while (clipper.Step()) {
            auto it = m_logs->begin();
            std::advance(it, clipper.DisplayStart);
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                ImGui::PushID(row);
                const auto& msg = *it;

                bool remove = drawLogItem(winSizeX, drawList, msg, midnightTime);
                if (remove) {
                    it = m_logs->erase(it);
                } else {
                    it++;
                }

                ImGui::PopID();
            }
        }
        clipper.End();
        ImGui::PopStyleVar(2);
        ImGui::EndChild();
    }

    bool LogPanel::drawLogItem(float winSizeX, ImDrawList* drawList, const Ref<AstraMessage>& msg, const std::chrono::system_clock::time_point& midnightTime) {
        bool ret = false;

        ImGui::SetNextItemAllowOverlap();
        ImGui::Dummy(ImVec2(winSizeX, 50));
        auto rectMin = ImGui::GetItemRectMin();
        auto rectMax = ImGui::GetItemRectMax();

        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(57, 59, 64, 255), 15);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)) {
            auto pos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(winSizeX - 30, pos.y - 53));
            if (UI::Core::Widgets::IconButton(ICON_FA_TRASH_CAN)) {
                ret = true;
            }
            ImGui::SetCursorPos(pos);
        }

        Ref<Core::Texture> icon;
        switch (msg->type) {
            case AstraMessageType::Error:
                icon = ViewerResources::ErrorIcon;
                break;
            case AstraMessageType::Warning:
                icon = ViewerResources::WarningIcon;
                break;
            case AstraMessageType::Success:
                icon = ViewerResources::SuccessIcon;
                break;
            case AstraMessageType::Info:
                icon = ViewerResources::InfoIcon;
                break;
            default:
                icon = ViewerResources::DebugIcon;
                break;
        }

        drawList->AddImage(icon->textureId(), rectMin + ImVec2(20, 10), rectMin + ImVec2(50, 40));

        if (msg->getTitle().empty()) {
            drawList->AddText(rectMin + ImVec2(70, 14), IM_COL32(255, 255, 255, 255), msg->getContent().c_str());
        } else {
            drawList->AddText(rectMin + ImVec2(70, 5), IM_COL32(255, 255, 255, 255), msg->getTitle().c_str());
            drawList->AddText(rectMin + ImVec2(70, 23), IM_COL32(100, 100, 100, 255), msg->getContent().c_str());
        }

        if (midnightTime > msg->getTiming()) {
            drawList->AddText(rectMax - ImVec2(145, 35), IM_COL32(130, 130, 130, 255),  std::format("{:%Y-%m-%d}", msg->getTiming()).c_str());
        } else {
            drawList->AddText(rectMax - ImVec2(100, 35), IM_COL32(130, 130, 130, 255),  std::format("{:%H:%M}", msg->getTiming()).c_str());
        }

        return ret;
    }

    void LogPanel::OnEvent(UI::Core::AEvent& pEvent) {
        if (pEvent.GetEventType() == UI::Core::EventType::ProjectLoaded) {
            m_logs = &AstraProject::CurrentProject()->GetData().logMessages;
        } else if (pEvent.GetEventType() == UI::Core::EventType::Notification) {
            if (!m_logs) {
                return;
            }

            const auto& msg = dynamic_cast<const UI::Core::NotificationEvent&>(pEvent).GetMessage();
            m_logs->emplace_front(msg);
            pEvent.Handled = true;
        }
    }
}
