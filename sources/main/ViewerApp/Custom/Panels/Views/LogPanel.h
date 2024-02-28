//
// Created by pierr on 24/03/2023.
//
#pragma once

#include <vector>
#include <chrono>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "CpuEngine/manager/LogManager.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class LogPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = ICON_FA_BELL " Log";

        LogPanel();

        void OnUpdate(const UI::Core::FrameInfo& pFrameInfo) override;

        void OnEvent(UI::Core::AEvent& pEvent) override;

    private:
        CPU::Core::LogManager& m_cpuLog = CPU::Core::LogManager::Get();

        std::list<Ref<AstraMessage>>* m_logs = nullptr;

        void showTableLogs();

        void showMenuBar();
        void drawPanelContent() override;
        static bool drawLogItem(float winSizeX, ImDrawList* drawList, const Ref<AstraMessage>& msg, const std::chrono::system_clock::time_point& currentTime);
    };

}
