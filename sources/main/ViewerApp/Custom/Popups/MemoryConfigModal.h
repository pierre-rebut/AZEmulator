//
// Created by pierr on 27/08/2023.
//

#pragma once

#include "EngineLib/data/Base.h"
#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "CpuEngine/manager/buses/DataBus.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/MultiInputModal.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class MemoryConfigModal : public UI::Core::AModal
    {
    public:
        static constexpr const char* NAME = ICON_FA_LINK " Devices connections";

    private:
        Ref<CPU::Core::DataBus> m_dataBus;
        Ref<CPU::Core::Device> m_selected;

        UI::Core::MultiInputModal m_inputModal{"New device"};

    public:
        MemoryConfigModal() : UI::Core::AModal(NAME) {}

        void OpenEdit(const Ref<CPU::Core::DataBus>& bus);

        void Reset() override;

    private:
        void drawPopupContent() override;
        void connectNewDevice();
        void drawConnectedDevicesTable();
        bool drawDeviceTableItem(const CPU::Core::ConnectedDevice& device);
    };

} // Astra
