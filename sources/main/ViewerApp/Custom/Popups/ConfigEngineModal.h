//
// Created by pierr on 31/07/2023.
//
#pragma once

#include "EngineLib/data/Base.h"
#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/manager/buses/DataBusManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "CpuEngine/engine/hardwareDevices/impl/ScreenDevice.h"

namespace Astra::UI::App {

    class ConfigEngineModal : public UI::Core::AModal
    {
    public:
        static constexpr const char* NAME = ICON_FA_WRENCH " Configure";

    private:
        Ref<CPU::Core::CpuEngine> m_engine;

        Ref<CPU::Core::CpuEngine> m_tmpDeviceConnection = nullptr;

    public:
        ConfigEngineModal() : UI::Core::AModal(NAME) {}

        void OpenEdit(const Ref<CPU::Core::CpuEngine>& engine);

        void Reset() override {
            m_engine = nullptr;
            m_tmpDeviceConnection = nullptr;
        }

    private:
        void drawPopupContent() override;

        void drawCpuMain(bool isCpuRunning) const;
        void drawCpuBinding(bool isCpuRunning) const;
        static void drawDevConfigPopup(const CPU::Core::DataBusManager& busManager, const CPU::Core::DevicesManager& devManager,
                                       const CPU::Core::DeviceValue* dev);


        void drawInterruptionConfig(bool isCpuRunning);
        void drawDeviceInterruptAdd();
        void drawDeviceInterruptConnections();
        void drawNameInput();

        void drawHardParamConfig(bool isCpuRunning);
        void updateHardParameters();
    };

}
