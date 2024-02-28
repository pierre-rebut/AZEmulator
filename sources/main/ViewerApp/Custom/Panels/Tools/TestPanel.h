//
// Created by pierr on 16/03/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APanel.h"

namespace Astra::UI::App {

    class TestPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = "TestPanel";

        TestPanel();

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override;

    protected:
        void drawPanelContent() override { /* do nothing */ }
    };

}
