//
// Created by pierr on 30/10/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "CpuEngine/manager/running/RunExecDebug.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class DebugPanel : public Core::APanel
    {
    public:
        static constexpr const char* NAME = ICON_FA_BUG " Debug";

    private:
        int m_fastSpeedValue = 1;

    public:

        DebugPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

    private:
        void drawPanelContent() override;
        void drawLogTable(CPU::Core::RunExecDebug* debugExec) const;
        static void drawLogItem(size_t instNum, const std::vector<int>& logItem) ;
        void drawFastSpeed();
    };

}
