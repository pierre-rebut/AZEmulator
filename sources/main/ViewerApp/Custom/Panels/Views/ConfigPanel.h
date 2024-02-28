//
// Created by pierr on 19/07/2023.
//

#pragma once

#include <array>
#include <tuple>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/SimpleInputModal.h"

#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/Custom/Popups/ConfigEngineModal.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/engine/hardwareDevices/impl/AudioDevice.h"
#include "ViewerApp/CoreLib/Resources/Sound.h"
#include "ViewerApp/Custom/Popups/ConfigDeviceModal.h"

namespace Astra::UI::App {
    class ConfigPanel : public UI::Core::APanel
    {
    private:
        UI::Core::SimpleInputModal m_inputModal{ICON_FA_MICROCHIP " New engine"};

        ConfigEngineModal* m_configEngineModal = UI::Core::WindowsManager::Get().getPopups().get<ConfigEngineModal>();
        ConfigDeviceModal* m_configDeviceModal = UI::Core::WindowsManager::Get().getPopups().get<ConfigDeviceModal>();

    public:
        static constexpr const char* NAME = ICON_FA_WRENCH " Config";

        static constexpr std::tuple<CPU::Core::DeviceType, const char*, const char*> DeviceTypesDisplay[] = {
                {CPU::Core::DeviceType::ENGINE,   "ENGINE",   ICON_FA_MICROCHIP},
                {CPU::Core::DeviceType::KEYBOARD, "KEYBOARD", ICON_FA_KEYBOARD},
                {CPU::Core::DeviceType::MOUSE,    "MOUSE",    ICON_FA_COMPUTER_MOUSE},
                {CPU::Core::DeviceType::SCREEN,   "SCREEN",   ICON_FA_DISPLAY},
                {CPU::Core::DeviceType::AUDIO,    "AUDIO",    ICON_FA_VOLUME_LOW},
                {CPU::Core::DeviceType::SERIAL,   "SERIAL",   ICON_FA_TERMINAL},
                {CPU::Core::DeviceType::DISK,     "DISK",     ICON_FA_HARD_DRIVE},
        };

        ConfigPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

        void OnEvent(Core::AEvent& pEvent) override;

        static std::string convertDeviceTypeToDisplay(CPU::Core::DeviceType type);

    private:
        void drawPanelContent() override;
        void addEngine();
        void drawEngineTableItem(int i, const Ref<CPU::Core::CpuEngine>& engine) const;
        static void drawSpeedPopup(const Ref<CPU::Core::CpuEngine>& sharedPtr);
        void drawEngineTable() const;

        static void dropLoadCore(const Ref<CPU::Core::CpuEngine>& engine);
        void drawAddPopup() const;

        void addDeviceButton();
        void drawDevicesTable();
        bool drawDeviceTableItem(const Ref<CPU::Core::HardwareDevice>& device);
        static void dropDeviceFile(const Ref<CPU::Core::Device>& device);
    };
}