//
// Created by pierr on 17/03/2023.
//
#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include "ViewerApp/CoreLib/Colors.h"
#include "ViewerApp/CoreLib/Graphics/AWindow.h"
#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "Commons/utils/ObjectStack.h"

namespace Astra::UI::Core {

    class TitleBar
    {
    public:
        static constexpr const char* NAME = "TitleBar";

        static constexpr const ImU32 HOVERED_COLUMN = IM_COL32(0, 0, 0, 80);
        static constexpr const float buttonsAreaWidth = 110;
        static constexpr const float titleBarHeight = 57.0f;
        static constexpr ImU32 buttonColP = UI::Core::Colors::textDarker;
        static constexpr const float buttonWidth = 18.0f;
        static constexpr const float buttonHeight = 18.0f;

    private:
        float moveOffsetX = 0;
        float moveOffsetY = 0;

    public:
        virtual ~TitleBar() = default;

        void DrawTitleBar();

    private:
        virtual void drawMenuItems(bool& menuOpen) = 0;
        virtual void drawCustomStatus() {}
        virtual void drawProjectNameInfo() {}

        void drawMenubar(const ImVec2& windowPadding);
        void drawProjectName(const ImVec2& windowPadding);

        static void drawWindowButton() ;

        void drawDragZone(float w, const ImGuiWindow* rootWindow, float windowWidth);

    };

}
