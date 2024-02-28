//
// Created by pierr on 30/10/2023.
//
#include "DebugPanel.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "CpuEngine/manager/running/RunExecDebug.h"
#include "Commons/utils/Utils.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TipsText.h"
#include "ViewerApp/CoreLib/AsyncJob.h"

namespace Astra::UI::App {

    void DebugPanel::drawPanelContent() {
        TipsText::Get().Show("DEBUG_PANEL_INFO");

        auto& runManager = CPU::Core::RunManager::Get();

        if (!AstraProject::CurrentProject()->getCurrentEngine()) {
            ImGui::Text(I18N::Get("NO_ENGINE_SELECTED"));
            return;
        }

        bool isModeDebug = runManager.GetRunMode();
        if (UI::Core::Toggle(I18N::Get("ENABLE_DEBUG"), isModeDebug)) {
            runManager.SetRunMode(isModeDebug);
        }

        drawFastSpeed();

        ImGui::BeginChild("DebugView", ImVec2(0, 0), true);
        if (runManager.GetRunMode()) {
            auto debugRunner = (CPU::Core::RunExecDebug*) runManager.GetRunner();
            drawLogTable(debugRunner);
        } else {
            ImGui::Text(I18N::Get("DEBUG_DISABLED"));
        }
        ImGui::EndChild();
    }

    void DebugPanel::drawFastSpeed() {
        ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 290);

        ImGui::Text(I18N::Get("FAST_STEP"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110);
        if (ImGui::InputInt("##fastStep", &m_fastSpeedValue, 0, 0) && m_fastSpeedValue < 1) {
            m_fastSpeedValue = 1;
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, Core::Colors::compliment);
        if (ImGui::Button(ICON_FA_FORWARD_STEP)) {
            Core::AsyncJob::Get().PushTask([this](){
                if (const auto currentEngine = AstraProject::CurrentProject()->getCurrentEngine()) {
                    CPU::Core::RunManager::Get().ExecuteStep(currentEngine, m_fastSpeedValue);
                }
            });
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text(ICON_FA_CIRCLE_QUESTION);
        Core::SetTooltip(I18N::Get("FAST_STEP_INFO"), -1);
    }

    void DebugPanel::drawLogTable(CPU::Core::RunExecDebug* debugRunner) const {
        static const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                             ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable;

        std::scoped_lock guard(debugRunner->GetDebugMutex());

        const auto& logs = debugRunner->GetDebugLog();
        const auto& logsHeader = AstraProject::CurrentProject()->getCurrentEngine()->GetCoreConfigInfo();

        if (!logsHeader || logs.empty()) {
            ImGui::Text(I18N::Get("NO_LOG"));
            return;
        }

        auto instNum = debugRunner->GetInstructionCount();

        if (!ImGui::BeginTable("DebugTable", logsHeader->debugHeader.size() + 1, flags)) {
            return;
        }

        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn(I18N::Get("ROW"));

        for (const auto& logItem: logsHeader->debugHeader) {
            ImGui::TableSetupColumn(logItem);
        }

        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin((int) logs.size());
        while (clipper.Step()) {
            auto it = logs.cbegin();
            std::advance(it, clipper.DisplayStart);

            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++, it++) {
                const auto& logEntry = *it;
                ImGui::PushID(row);

                drawLogItem(instNum - row, logEntry);

                ImGui::PopID();
                ImGui::Separator();
            }
        }
        clipper.End();
        ImGui::EndTable();
    }

    void DebugPanel::drawLogItem(size_t instNum, const std::vector<int>& logItem) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%d", instNum);

        for (const auto& var: logItem) {
            ImGui::TableNextColumn();
            ImGui::Text("%x", var);
        }
    }
}
