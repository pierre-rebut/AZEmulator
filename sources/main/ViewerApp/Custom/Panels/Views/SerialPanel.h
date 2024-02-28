//
// Created by pierr on 31/10/2023.
//
#pragma once

#include <chrono>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "CpuEngine/engine/hardwareDevices/impl/SerialDevice.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class SerialPanel : public Core::APanel
    {
    private:
        bool m_isWindowFocused = false;
        std::string m_currentMessage;
        std::chrono::time_point<std::chrono::system_clock> m_startTime;

        Ref<CPU::Core::SerialDevice> m_serialDevice = nullptr;

    public:
        static constexpr const char* NAME = ICON_FA_TERMINAL " Serial";

        SerialPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

        void OnEvent(Core::AEvent& pEvent) override;
        void OnUpdate(const Core::FrameInfo& pFrameInfo) override;

    private:
        void drawPanelContent() override;
        void drawMessageHistory();
        void drawSerialInput();
        void enterCharacter(unsigned short c);
        void drawCursor();
    };

}
