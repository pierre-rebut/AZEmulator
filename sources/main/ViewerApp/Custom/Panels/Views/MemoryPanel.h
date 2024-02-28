//
// Created by pierr on 16/03/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APanel.h"

#include "CpuEngine/manager/buses/DataBusManager.h"
#include "CpuEngine/engine/hardwareDevices/RamMemory.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/SimpleInputModal.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/MultiInputModal.h"
#include "ViewerApp/Custom/Popups/MemoryConfigModal.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/SimpleQuestionModal.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"

namespace Astra::UI::App {

    class MemoryPanel : public UI::Core::APanel
    {
    private:
        std::filesystem::path tmpDropFile;
        Ref<CPU::Core::CpuEngine> m_currentEngine;

        UI::Core::MultiInputModal m_inputsModal{"Memory config"};
        UI::Core::SimpleInputModal m_inputModal{ICON_FA_PEN " Rename memory"};
        UI::Core::SimpleQuestionModal m_questionModal{"Memory"};
        MemoryConfigModal* m_configModal = UI::Core::WindowsManager::Get().getPopups().get<MemoryConfigModal>();

        void drawHexTable(const Ref<CPU::Core::DataBus>& dataBus, bool isCpuRunning);

        unsigned int m_searchBuffer = 0;
        bool m_searchUpdated = false;

    public:
        static constexpr const char* NAME = ICON_FA_MEMORY " Memories";

        MemoryPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

    private:
        void OnEvent(UI::Core::AEvent& pEvent) override;

        void drawPanelContent() override;

        void
        drawHexTableItem(const Ref<CPU::Core::DataBus>& dataBus, bool pIsRunning, int currentPCAddr, int addr, std::list<CPU::Core::ConnectedDevice>::const_iterator& it,
                         std::ostringstream& asciiValue);

        void drawMenuBar(const Ref<CPU::Core::DataBus>& dataBus, bool isCpuRunning);
        void dragDropTarget(const Ref<CPU::Core::DataBus>& dataBus, bool isCpuRunning);
        void openQuestionLoadMemory(const Ref<CPU::Core::DataBus>& dataBus);
        void drawMemTab();
        static BYTE drawRamCell(const Ref<CPU::Core::DataBus>& dataBus, bool pIsRunning, int currentAddr, BYTE currentVal) ;
    };

}
