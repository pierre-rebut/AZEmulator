//
// Created by pierr on 16/08/2023.
//

#pragma once

#include "ViewerApp/CoreLib/MainScreen.h"

namespace Astra::UI::App {

    class CustomMainScreen : public UI::Core::MainScreen
    {
    public:
        void init() override;
        void displayInitScreen(const UI::Core::FrameInfo& pFrameInfo) override;
        void setWindowFocusStartUp() const override;

    private:
        void defineCustomDockspace(ImGuiID dockMainId) const override;

        static void initPanels() ;
        static void initModals() ;
    };

} // Astra
