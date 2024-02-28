//
// Created by pierr on 02/08/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "Commons/utils/UUID.h"
#include "CpuEngine/engine/Device.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class SettingsModal : public UI::Core::AModal
    {
    public:
        static constexpr const char* NAME = ICON_FA_GEARS " Settings";

        SettingsModal() : AModal(NAME) {}

    private:
        void drawPopupContent() override;
        void drawGlobalSettings() const;
        void drawProjectSettings() const;
    };

}
