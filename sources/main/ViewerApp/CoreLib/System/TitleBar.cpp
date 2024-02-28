//
// Created by pierr on 17/03/2023.
//
#include "TitleBar.h"

#include "Commons/Profiling.h"

#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/CoreLib/Project.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace Astra::UI::Core {

    void TitleBar::DrawTitleBar() {
        ENGINE_PROFILE_FUNCTION();

        const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

        ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y));
        const ImVec2 titlebarMin = ImGui::GetCursorScreenPos();
        const ImVec2 titlebarMax = {
                titlebarMin.x + ImGui::GetWindowWidth() - windowPadding.y * 2.0f,
                titlebarMin.y + titleBarHeight
        };
        auto* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(titlebarMin, titlebarMax, Colors::titlebar);

        // Logo
        ImGui::SetCursorPos({16.0f + windowPadding.x, 8.0f + windowPadding.y});
        ImGui::Image(App::ViewerResources::LogoTexture->textureId(), App::ViewerResources::LogoTexture->textureSize());

        const float w = ImGui::GetContentRegionAvail().x;

        // Title bar drag area
        const auto* rootWindow = ImGui::GetCurrentWindow()->RootWindow;
        const float windowWidth = rootWindow->RootWindow->Size.x;

        drawMenubar(windowPadding);
        drawProjectName(windowPadding);

        ImGui::SetCursorPos(ImVec2{
                ImGui::GetWindowWidth() / 2,
                titleBarHeight / 4 + (windowPadding.y / 2)
        });

        ImGui::BeginChild("##customStatus", ImVec2(200,40), false, ImGuiWindowFlags_NoBackground);
        drawCustomStatus();
        ImGui::EndChild();

        drawWindowButton();
        drawDragZone(w, rootWindow, windowWidth);
    }

    void TitleBar::drawMenubar(const ImVec2& windowPadding) {
        ENGINE_PROFILE_FUNCTION();

        const float logoOffset = 16.0f * 2.0f + 41.0f + windowPadding.x;
        ImGui::SetCursorPos(ImVec2(logoOffset, 4.0f));

        const ImRect menuBarRect = {
                ImGui::GetCursorPos(), {ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeightWithSpacing()}
        };

        if (UI::Core::BeginMenuBar(menuBarRect)) {
            bool menuOpen = ImGui::IsPopupOpen("##menubar", ImGuiPopupFlags_AnyPopupId);

            if (menuOpen) {
                const ImU32 colActive = ColourWithSaturation(UI::Core::Colors::accent, 0.5f);
                ImGui::PushStyleColor(ImGuiCol_Header, colActive);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colActive);
            }

            drawMenuItems(menuOpen);

            if (menuOpen) {
                ImGui::PopStyleColor(2);
            }

            EndMenuBar();
        }
    }


    void TitleBar::drawDragZone(const float w, const ImGuiWindow* rootWindow, const float windowWidth) {
        ENGINE_PROFILE_FUNCTION();
        ImGui::SetCursorPos(ImVec2(0, 0));
        if (ImGui::InvisibleButton("##titleBarDragZone", ImVec2(w - buttonsAreaWidth, titleBarHeight),
                                   ImGuiButtonFlags_PressedOnClick)) {
            ImVec2 point = ImGui::GetMousePos();
            ImRect rect = rootWindow->Rect();
            // Calculate the difference between the cursor pos and window pos
            moveOffsetX = point.x - rect.Min.x;
            moveOffsetY = point.y - rect.Min.y;
        }

        const AWindow& window = CoreEngine::Get().GetWindow();

        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
            if (window.isWindowMaximized()) {
                window.restoreWindow();
            } else {
                window.maximizeWindow();
            }
        } else if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (window.isWindowMaximized()) {
                window.restoreWindow();

                int newWidth;
                int newHeight;
                window.getWindowSize(newWidth, newHeight);

                // Offset position proportionally to mouse position on titlebar
                // This ensures we dragging window relatively to cursor position on titlebar
                // correctly when window size changes
                if (windowWidth - (float) newWidth > 0.0f) {
                    moveOffsetX *= (float) newWidth / windowWidth;
                }
            }

            ImVec2 point = ImGui::GetMousePos();
            window.setWindowPos(point.x - moveOffsetX, point.y - moveOffsetY);
        }
    }

    void TitleBar::drawWindowButton() {
        ENGINE_PROFILE_FUNCTION();

        const AWindow& window = CoreEngine::Get().GetWindow();

        const ImU32 buttonColN = ColourWithMultipliedValue(UI::Core::Colors::text, 0.9f);
        const ImU32 buttonColH = ColourWithMultipliedValue(UI::Core::Colors::text, 1.2f);

        // Minimize Button
        {
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonsAreaWidth);
            ShiftCursorY(8.0f);
            const auto iconSize = App::ViewerResources::MinimizeIcon->textureSize();
            const float padY = (buttonHeight - iconSize.y) / 2.0f;
            if (ImGui::InvisibleButton("Minimize", ImVec2(buttonWidth, buttonHeight))) {
                window.iconifyWindow();
            }

            DrawButtonImage(App::ViewerResources::MinimizeIcon, buttonColN, buttonColH, buttonColP,
                            RectExpanded(UI::Core::GetItemRect(), 0.0f, -padY));
        }

        // Maximize Button
        {
            ImGui::SameLine(0, 15.0f);
            ShiftCursorY(8.0f);
            bool isMaximized = window.isWindowMaximized();

            if (ImGui::InvisibleButton("Maximize", ImVec2(buttonWidth, buttonHeight))) {
                if (isMaximized) {
                    window.restoreWindow();
                } else {
                    window.maximizeWindow();
                }
            }

            DrawButtonImage(isMaximized ? App::ViewerResources::RestoreIcon : App::ViewerResources::MaximizeIcon, buttonColN,
                            buttonColH, buttonColP);
        }

        // Close Button
        {
            ImGui::SameLine(0, 15.0f);
            ShiftCursorY(8.0f);
            if (ImGui::InvisibleButton("Close", ImVec2(buttonWidth, buttonHeight))) {
                Events::Get().OnEvent<WindowCloseEvent>();
            }
            DrawButtonImage(App::ViewerResources::CloseIcon, Colors::text, IM_COL32(255, 0, 0, 255), buttonColP);
        }
    }

    void TitleBar::drawProjectName(const ImVec2& windowPadding) {
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::GetWindowWidth() < 1200) {
            return;
        }

        const char* projectName = "<undefined>";
        if (const auto project = Project::CurrentProject()) {
            projectName = project->getProjectName().c_str();
        }

        const float rightOffset = ImGui::GetWindowWidth() / 5.0f;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - rightOffset - ImGui::CalcTextSize(projectName).x - 20);
        ShiftCursorY(1.0f + windowPadding.y);

        const auto minRect = ImVec2(ImGui::GetWindowPos().x + ImGui::GetCursorPosX(), ImGui::GetCursorPosY());

        ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::BOLD]);
        ImGui::Text(projectName);

        drawProjectNameInfo();

        ScopedColour border(ImGuiCol_Border, IM_COL32(40, 40, 40, 255));
        DrawBorder(UI::Core::RectExpanded(ImRect(minRect, GetItemRect().Max), 24.0f, 68.0f), 1.0f, 3.0f, 0.0f, -60.0f);
    }
}
