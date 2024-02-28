//
// Created by pierr on 02/08/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/Custom/Popups/ConfigEngineModal.h"

namespace Astra::UI::App {

    struct FrequencyData {
        size_t plotIndex = 0;
        std::array<float, 50> plot{};

        FrequencyData() {
            plot.fill(0);
        }
    };

    class StatisticsPanel : public UI::Core::APanel
    {
    private:
        double m_updateSystemSpeed = 0;
        FrequencyData m_systemData;
        std::unordered_map<UUID, FrequencyData> m_enginesData;

    public:
        static constexpr const char* NAME = ICON_FA_CHART_LINE " Statistics";
        StatisticsPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

    private:
        void drawPanelContent() override;

        void OnUpdate(const Core::FrameInfo& pFrameInfo) override;
        void OnEvent(Core::AEvent& event) override;
    };

}
