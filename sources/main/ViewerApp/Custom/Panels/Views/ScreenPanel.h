//
// Created by pierr on 18/03/2023.
//
#pragma once

#include "EngineLib/data/Base.h"
#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "ViewerApp/CoreLib/Resources/Texture.h"

#include "CpuEngine/engine/hardwareDevices/impl/ScreenDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/KeyboardDevice.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/engine/hardwareDevices/impl/AudioDevice.h"
#include "ViewerApp/CoreLib/Resources/Sound.h"
#include "CpuEngine/engine/hardwareDevices/impl/MouseDevice.h"

namespace Astra::UI::App {

    class ScreenPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = ICON_FA_DISPLAY " Screen";

    private:
        bool m_isFocus = false;
        ImVec2 m_screenPos;
        ImVec2 m_screenSize;

        Scope<UI::Core::Texture> m_screenTexture = nullptr;
        Scope<UI::Core::Sound> m_sound = nullptr;

        Ref<CPU::Core::KeyboardDevice> m_keyboardDevice = nullptr;
        Ref<CPU::Core::MouseDevice> m_mouseDevice = nullptr;
        Ref<CPU::Core::ScreenDevice> m_screenDevice = nullptr;
        Ref<CPU::Core::AudioDevice> m_audioDevice = nullptr;

    public:
        ScreenPanel();

        void OnUpdate(const Core::FrameInfo& pFrameInfo) override;

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override;

        void OnEvent(UI::Core::AEvent& pEvent) override;

    private:
        void drawPanelContent() override;
    };

}
