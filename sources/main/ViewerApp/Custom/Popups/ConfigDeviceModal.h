//
// Created by pierr on 16/01/2024.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/ScreenDevice.h"

namespace Astra::UI::App {

    class ConfigDeviceModal : public UI::Core::AModal
    {
    public:
        static constexpr const char* NAME = ICON_FA_WRENCH " Device configure";

    private:
        Ref<CPU::Core::HardwareDevice> m_device;

    public:
        ConfigDeviceModal() : UI::Core::AModal(NAME) {}
        void OpenEdit(const Ref<CPU::Core::HardwareDevice>& device);

        void Reset() override {
            m_device = nullptr;
        }

    private:
        void drawPopupContent() override;

        void drawDeviceConfig();
        void drawNameInput();

        void drawDiskDeviceConfig();
        void drawScreenDeviceConfig();
        void drawAudioDeviceConfig();
        static void applyNewScreenSettings(const Ref<CPU::Core::ScreenDevice>& screenDevice, int width, int height) ;
    };

}
