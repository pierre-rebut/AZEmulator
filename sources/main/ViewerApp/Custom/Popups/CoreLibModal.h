//
// Created by pierr on 28/08/2023.
//

#pragma once

#include "EngineLib/data/Base.h"
#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "CpuEngine/manager/cpulib/lib/CoreLib.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/manager/cpulib/CoreLibManager.h"

namespace Astra::UI::App {

    class CoreLibModal : public UI::Core::AModal
    {
    public:
        static constexpr const char* NAME = ICON_FA_BOOK " Core library";

        CoreLibModal() : UI::Core::AModal(NAME) {}

    private:
        void drawPopupContent() override;
        static void drawCoreLibs(const std::map<std::string, Scope<CPU::Core::CoreLibDir>>& coreLibs);
        static void drawCoreLibsItem(int i, const Ref<CPU::Core::CoreLib>& lib) ;
        static void drawSubMenu();
    };

} // Astra
