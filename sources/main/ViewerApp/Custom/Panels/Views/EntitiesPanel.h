//
// Created by pierr on 16/03/2023.
//
#pragma once

#include <vector>
#include <string>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "EngineLib/data/Types.h"
#include "Commons/utils/UUID.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/SimpleInputModal.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class EntitiesPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = ICON_FA_MAGNIFYING_GLASS " Entities";

        EntitiesPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {}

        void OnEvent(UI::Core::AEvent& pEvent) override;

    private:
        Ref<CPU::Core::CpuEngine> m_currentEngine;

        void drawPanelContent() override;

        void drawSubMenu(bool isCpuRunning) const;
        void dragDropTargetEntitiesConfig(bool isRunning) const;

        void drawRegistersTable(bool pIsRunning, const CPU::Core::RegistersInfo& registersList) const;
        static void drawRegistersTableItem(int i, const std::string& regName, CPU::Core::IRegister* reg, bool isRunning);

        void drawFlagsTable(bool pIsRunning) const;
        static void drawFlagsTableItem(int i, const std::string& flagName, CPU::Core::IFlagRegister& flagValue, bool isRunning);

        void saveEntitiesValue() const;
        void loadEntitiesValue() const;
    };

}
