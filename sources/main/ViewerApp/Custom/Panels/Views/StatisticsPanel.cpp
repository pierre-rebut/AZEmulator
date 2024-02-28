//
// Created by pierr on 02/08/2023.
//
#include "StatisticsPanel.h"

#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/manager/running/RunManager.h"

#include "imgui_plot/implot.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TipsText.h"

namespace Astra::UI::App {
    void StatisticsPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        TipsText::Get().Show("STAT_PANEL_INFO");

        const auto& runManager = CPU::Core::RunManager::Get();

        ImGui::Text(myFormat(ICON_FA_CLOCK " {} : {}", I18N::Get("SYSTEM_CLOCK"), runManager.GetElapsedSystemClockCount()).c_str());
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Total : 0x%llX", runManager.GetSystemClockCount());
        }

        if (ImPlot::BeginPlot("##systemClock")) {
            ImPlot::SetupAxes(nullptr, I18N::Get("FREQUENCY"), 0, ImPlotAxisFlags_AutoFit);
            ImPlot::SetupFinish();

            ImPlot::PlotLine("##clock", m_systemData.plot.data(), m_systemData.plot.size(), 1, 0, 0, m_systemData.plotIndex);

            ImPlot::EndPlot();
        }

        ImGui::SeparatorText(myFormat(ICON_FA_MICROCHIP " {}", I18N::Get("ENGINES")).c_str());
        if (ImPlot::BeginPlot("##engines")) {
            ImPlot::SetupAxes(nullptr, I18N::Get("FREQUENCY"), 0, ImPlotAxisFlags_AutoFit);
            ImPlot::SetupFinish();

            for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                if (!engine->IsRunnable()) {
                    continue;
                }

                const auto& enginePlot = m_enginesData[engineUUID];
                ImPlot::PlotLine(engine->GetName().c_str(), enginePlot.plot.data(), enginePlot.plot.size(), 1, 0, 0, enginePlot.plotIndex);
            }
            ImPlot::EndPlot();
        }

        ImGui::SeparatorText(myFormat(ICON_FA_VOLUME_LOW " {}", I18N::Get("AUDIO")).c_str());
        ImGui::Text(I18N::Get("NOT_IMPLEMENTED"));
    }

    void StatisticsPanel::OnUpdate(const Core::FrameInfo& pFrameInfo) {
        auto& runManager = CPU::Core::RunManager::Get();

        if (!runManager.isRunning()) {
            return;
        }

        if (ImGui::GetTime() - m_updateSystemSpeed >= 1) {

            runManager.UpdateElapsedCycle();
            for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                engine->GetRunService()->UpdateElapsedCycle();
            }

            m_systemData.plot[m_systemData.plotIndex] = (float) runManager.GetElapsedSystemClockCount();
            m_systemData.plotIndex = (m_systemData.plotIndex + 1) % m_systemData.plot.size();

            for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                if (!engine->IsRunnable()) {
                    continue;
                }

                auto& enginePlot = m_enginesData[engineUUID];
                enginePlot.plot[enginePlot.plotIndex] = (float) engine->GetRunService()->GetElapsedCycle();
                enginePlot.plotIndex = (enginePlot.plotIndex + 1) % enginePlot.plot.size();
            }

            m_updateSystemSpeed = ImGui::GetTime();
        }
    }

    void StatisticsPanel::OnEvent(Core::AEvent& pEvent) {
        switch (pEvent.GetEventType()) {
            case UI::Core::EventType::ProjectLoaded: {
                m_systemData.plot.fill(0);
                m_enginesData.clear();
                break;
            }
            default:
                break;
        }
    }
}
