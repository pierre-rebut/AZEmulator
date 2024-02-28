//
// Created by pierr on 21/03/2023.
//
#include "Utils.h"
#include "Commons/Log.h"
#include "Commons/Profiling.h"
#include "Commons/utils/Utils.h"
#include "Commons/AstraException.h"
#include "IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/System/I18N.h"

#include <imgui_stdlib.h>

namespace Astra::UI::Core {
    static constexpr float WINDOWS_HOVER_PADDING = 4.0f;                        // Extend outside window for hovering/resizing (maxxed with TouchPadding) and inside windows for borders. Affect FindHoveredWindow().
    static constexpr float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER = 0.04f;    // Reduce visual noise by only highlighting the border after a certain time.

    struct ImGuiResizeBorderDef
    {
        ImVec2 InnerDir;
        ImVec2 SegmentN1;
        ImVec2 SegmentN2;
        float OuterAngle = false;
    };

    static const std::array<ImGuiResizeBorderDef, 4> resizeBorderDef = {
            ImGuiResizeBorderDef(ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f), // Left
            ImGuiResizeBorderDef(ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f), // Right
            ImGuiResizeBorderDef(ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f), // Up
            ImGuiResizeBorderDef(ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f)  // Down
    };

    // Data for resizing from corner
    struct ImGuiResizeGripDef
    {
        ImVec2 CornerPosN;
        ImVec2 InnerDir;
        int AngleMin12;
        int AngleMax12;
    };
    static const std::array<ImGuiResizeGripDef, 4> resize_grip_def = {
            ImGuiResizeGripDef(ImVec2(1, 1), ImVec2(-1, -1), 0, 3),  // Lower-right
            ImGuiResizeGripDef(ImVec2(0, 1), ImVec2(+1, -1), 3, 6),  // Lower-left
            ImGuiResizeGripDef(ImVec2(0, 0), ImVec2(+1, +1), 6, 9),  // Upper-left (Unused)
            ImGuiResizeGripDef(ImVec2(1, 0), ImVec2(-1, +1), 9, 12)  // Upper-right (Unused)
    };

    bool BeginPopup(const char* strId, ImGuiWindowFlags flags) {
        bool opened = false;
        if (ImGui::BeginPopup(strId, flags)) {
            opened = true;

            const float padding = ImGui::GetStyle().WindowBorderSize;
            const ImRect windowRect = UI::Core::RectExpanded(ImGui::GetCurrentWindow()->Rect(), -padding, -padding);
            ImGui::PushClipRect(windowRect.Min, windowRect.Max, false);
            const ImColor col1 = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
            const ImColor col2 = UI::Core::ColourWithMultipliedValue(col1, 0.8f);
            ImGui::GetWindowDrawList()->AddRectFilledMultiColor(windowRect.Min, windowRect.Max, col1, col1, col2, col2);
            ImGui::GetWindowDrawList()->AddRect(windowRect.Min, windowRect.Max,
                                                UI::Core::ColourWithMultipliedValue(col1, 1.1f));
            ImGui::PopClipRect();

            // Popped in EndPopup()
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 80));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
        }

        return opened;
    }

    void EndPopup() {
        ImGui::PopStyleVar(); // WindowPadding
        ImGui::PopStyleColor(); // HeaderHovered
        ImGui::EndPopup();
    }

    // MenuBar which allows you to specify its rectangle
    bool BeginMenuBar(const ImRect& barRectangle) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        IM_ASSERT(!window->DC.MenuBarAppending);
        ImGui::BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
        ImGui::PushID("##menubar");

        const ImVec2 padding = window->WindowPadding;

        // We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
        // We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
        ImRect bar_rect = UI::Core::RectOffset(barRectangle, 0.0f, padding.y);
        ImRect clip_rect(IM_ROUND(bar_rect.Min.x + window->WindowBorderSize + window->Pos.x),
                         IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
                         IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x, bar_rect.Max.x + window->Pos.x -
                                                                        ImMax(window->WindowRounding,
                                                                              window->WindowBorderSize))),
                         IM_ROUND(bar_rect.Max.y + window->Pos.y));


        clip_rect.ClipWith(window->OuterRectClipped);
        ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

        // We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something analogous here, maybe a BeginGroupEx() with flags).
        window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->Pos.x,
                                                                bar_rect.Min.y + window->Pos.y);
        window->DC.LayoutType = ImGuiLayoutType_Horizontal;
        window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
        window->DC.MenuBarAppending = true;
        ImGui::AlignTextToFramePadding();
        return true;
    }

    void EndMenuBar() {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;
        ImGuiContext& g = *GImGui;

        // Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
        if (ImGui::NavMoveRequestButNoResultYet() &&
            (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right) &&
            (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu)) {
            // Try to find out if the request is for one of our child menu
            ImGuiWindow* nav_earliest_child = g.NavWindow;
            while (nav_earliest_child->ParentWindow &&
                   (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
                nav_earliest_child = nav_earliest_child->ParentWindow;
            if (nav_earliest_child->ParentWindow == window &&
                nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal &&
                (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0) {
                // To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
                // This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple window (probably not worth bothering)
                const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
                IM_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer)); // Sanity check
                ImGui::FocusWindow(window);
                ImGui::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
                g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
                g.NavDisableMouseHover = g.NavMousePosDirty = true;
                ImGui::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags,
                                             g.NavMoveScrollFlags); // Repeat
            }
        }

        IM_MSVC_WARNING_SUPPRESS(
                6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
        // IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar); // NOTE(Yan): Needs to be commented out because Jay
        IM_ASSERT(window->DC.MenuBarAppending);
        ImGui::PopClipRect();
        ImGui::PopID();
        window->DC.MenuBarOffset.x = window->DC.CursorPos.x -
                                     window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
        g.GroupStack.back().EmitItem = false;
        ImGui::EndGroup(); // Restore position on layer 0
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.MenuBarAppending = false;
    }

    static ImVec2 calcWindowSizeAfterConstraint(const ImGuiWindow* window, const ImVec2& sizeDesired) {
        ImGuiContext& g = *GImGui;
        ImVec2 newSize = sizeDesired;

        if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) {
            // Using -1,-1 on either X/Y axis to preserve the current size.
            ImRect cr = g.NextWindowData.SizeConstraintRect;
            newSize.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(newSize.x, cr.Min.x, cr.Max.x)
                                                         : window->SizeFull.x;
            newSize.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(newSize.y, cr.Min.y, cr.Max.y)
                                                         : window->SizeFull.y;
            if (g.NextWindowData.SizeCallback) {
                ImGuiSizeCallbackData data;
                data.UserData = g.NextWindowData.SizeCallbackUserData;
                data.Pos = window->Pos;
                data.CurrentSize = window->SizeFull;
                data.DesiredSize = newSize;
                g.NextWindowData.SizeCallback(&data);
                newSize = data.DesiredSize;
            }
            newSize.x = IM_FLOOR(newSize.x);
            newSize.y = IM_FLOOR(newSize.y);
        }

        // Minimum size
        if (!(window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize))) {
            const ImGuiWindow* windowForHeight = (window->DockNodeAsHost && window->DockNodeAsHost->VisibleWindow)
                                                 ? window->DockNodeAsHost->VisibleWindow : window;
            const float decoration_up_height =
                    windowForHeight->TitleBarHeight() + windowForHeight->MenuBarHeight();
            newSize = ImMax(newSize, g.Style.WindowMinSize);
            newSize.y = ImMax(newSize.y, decoration_up_height + ImMax(0.0f, g.Style.WindowRounding -
                                                                            1.0f)); // Reduce artifacts with very small windows
        }

        return newSize;
    }


    static ImRect getResizeBorderRect(const ImGuiWindow* window, int borderN, float perpPadding, float thickness) {
        ImRect rect = window->Rect();
        if (thickness == 0.0f) {
            rect.Max.x -= 1;
            rect.Max.y -= 1;
        }

        if (borderN == ImGuiDir_Left) {
            return {rect.Min.x - thickness, rect.Min.y + perpPadding, rect.Min.x + thickness,
                    rect.Max.y - perpPadding};
        }
        if (borderN == ImGuiDir_Right) {
            return {rect.Max.x - thickness, rect.Min.y + perpPadding, rect.Max.x + thickness,
                    rect.Max.y - perpPadding};
        }
        if (borderN == ImGuiDir_Up) {
            return {rect.Min.x + perpPadding, rect.Min.y - thickness, rect.Max.x - perpPadding,
                    rect.Min.y + thickness};
        }
        if (borderN == ImGuiDir_Down) {
            return {rect.Min.x + perpPadding, rect.Max.y - thickness, rect.Max.x - perpPadding,
                    rect.Max.y + thickness};
        }

        throw AstraException("Invalid border type {}", borderN);
    }

    static ImVec2 calcWindowAutoFitSize(const ImGuiWindow* window, const ImVec2& sizeContents) {
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const float decorationUpHeight = window->TitleBarHeight() + window->MenuBarHeight();

        ImVec2 sizePad = {
                window->WindowPadding.x * 2.0f,
                window->WindowPadding.y * 2.0f
        };
        ImVec2 sizeDesired = {
                sizeContents.x + sizePad.x + 0.0f,
                sizeContents.y + sizePad.y + decorationUpHeight
        };

        if (window->Flags & ImGuiWindowFlags_Tooltip) {
            // Tooltip always resize
            return sizeDesired;
        } else {
            // Maximum window size is determined by the viewport size or monitor size
            const bool isPopup = (window->Flags & ImGuiWindowFlags_Popup) != 0;
            const bool isMenu = (window->Flags & ImGuiWindowFlags_ChildMenu) != 0;
            ImVec2 sizeMin = style.WindowMinSize;
            if (isPopup ||
                isMenu) // Popups and menus bypass style.WindowMinSize by default, but we give then a non-zero minimum size to facilitate understanding problematic cases (e.g. empty popups)
                sizeMin = ImMin(sizeMin, ImVec2(4.0f, 4.0f));

            // FIXME-VIEWPORT-WORKAREA: May want to use GetWorkSize() instead of Size depending on the type of windows?
            ImVec2 availSize = window->Viewport->Size;
            if (window->ViewportOwned)
                availSize = ImVec2(FLT_MAX, FLT_MAX);

            const int monitorIdx = window->ViewportAllowPlatformMonitorExtend;
            if (monitorIdx >= 0 && monitorIdx < g.PlatformIO.Monitors.Size) {
                availSize = g.PlatformIO.Monitors[monitorIdx].WorkSize;
            }

            ImVec2 sizeAutoFit = ImClamp(
                    sizeDesired,
                    sizeMin,
                    ImMax(sizeMin, {availSize.x - style.DisplaySafeAreaPadding.x * 2.0f,
                                    availSize.y - style.DisplaySafeAreaPadding.y * 2.0f})
            );

            // When the window cannot fit all contents (either because of constraints, either because screen is too small),
            // we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than ViewportSize-WindowPadding.
            ImVec2 sizeAutoFitAfterConstraint = calcWindowSizeAfterConstraint(window, sizeAutoFit);
            bool willHaveScrollbarX = (sizeAutoFitAfterConstraint.x - sizePad.x - 0.0f < sizeContents.x &&
                                       !(window->Flags & ImGuiWindowFlags_NoScrollbar) &&
                                       (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) ||
                                      (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            bool willHaveScrollbarY =
                    (sizeAutoFitAfterConstraint.y - sizePad.y - decorationUpHeight < sizeContents.y &&
                     !(window->Flags & ImGuiWindowFlags_NoScrollbar)) ||
                    (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);

            if (willHaveScrollbarX)
                sizeAutoFit.y += style.ScrollbarSize;
            if (willHaveScrollbarY)
                sizeAutoFit.x += style.ScrollbarSize;
            return sizeAutoFit;
        }
    }

    static void calcResizePosSizeFromAnyCorner(const ImGuiWindow* window, const ImVec2& cornerTarget,
                                               const ImVec2& cornerNorm, ImVec2& outPos, ImVec2& outSize) {
        // Expected window upper-left
        ImVec2 posMin = ImLerp(cornerTarget, window->Pos, cornerNorm);
        // Expected window lower-right
        ImVec2 posMax = ImLerp({window->Pos.x + window->Size.x, window->Pos.y + window->Size.y},
                               cornerTarget, cornerNorm);
        ImVec2 sizeExpected = {posMax.x - posMin.x, posMax.y - posMin.y};
        ImVec2 sizeConstrained = calcWindowSizeAfterConstraint(window, sizeExpected);

        outPos = posMin;
        if (cornerNorm.x == 0.0f)
            outPos.x -= (sizeConstrained.x - sizeExpected.x);
        if (cornerNorm.y == 0.0f)
            outPos.y -= (sizeConstrained.y - sizeExpected.y);
        outSize = sizeConstrained;
    }

    // Exposed to be used for window with disabled decorations
    // This border is going to be drawn even if window border size is set to 0.0f
    void RenderWindowOuterBorders(ImGuiWindow* window) {

        const ImGuiContext& g = *GImGui;
        float rounding = window->WindowRounding;
        float borderSize = 1.0f;

        if (borderSize > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
            window->DrawList->AddRect(window->Pos, {window->Pos.x + window->Size.x, window->Pos.y + window->Size.y},
                                      ImGui::GetColorU32(ImGuiCol_Border), rounding, 0, borderSize);

        if (int borderHeld = window->ResizeBorderHeld; borderHeld != -1) {
            const ImGuiResizeBorderDef& def = resizeBorderDef[borderHeld];
            ImRect borderR = getResizeBorderRect(window, borderHeld, rounding, 0.0f);
            ImVec2 p1 = ImLerp(borderR.Min, borderR.Max, def.SegmentN1);
            const float offsetX = def.InnerDir.x * rounding;
            const float offsetY = def.InnerDir.y * rounding;
            p1.x += 0.5f + offsetX;
            p1.y += 0.5f + offsetY;

            ImVec2 p2 = ImLerp(borderR.Min, borderR.Max, def.SegmentN2);
            p2.x += 0.5f + offsetX;
            p2.y += 0.5f + offsetY;

            window->DrawList->PathArcTo(p1, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
            window->DrawList->PathArcTo(p2, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
            window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0,
                                         ImMax(2.0f, borderSize)); // Thicker than usual
        }
        if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive) {
            float y = window->Pos.y + window->TitleBarHeight() - 1;
            window->DrawList->AddLine(ImVec2(window->Pos.x + borderSize, y),
                                      ImVec2(window->Pos.x + window->Size.x - borderSize, y),
                                      ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameBorderSize);
        }
    }

    static void
    manualResizeGripHeld(ImGuiWindow* window, ImGuiContext& g, const float gripHoverInnerSize,
                         const ImRect& visibilityRect, ImVec2& posTarget, ImVec2& sizeTarget,
                         int borderN, int& borderHeld) {
        const ImGuiResizeBorderDef& def = resizeBorderDef[borderN];
        const ImGuiAxis axis = (borderN == ImGuiDir_Left || borderN == ImGuiDir_Right) ? ImGuiAxis_X
                                                                                       : ImGuiAxis_Y;

        bool hovered;
        bool held;

        ImRect borderRect = getResizeBorderRect(window, borderN, gripHoverInnerSize, WINDOWS_HOVER_PADDING);
        ImGuiID borderId = window->GetID(borderN + 4); // == GetWindowResizeBorderID()
        ImGui::ItemAdd(borderRect, borderId);
        ImGui::ButtonBehavior(borderRect, borderId, &hovered, &held, ImGuiButtonFlags_FlattenChildren);

        //GetForegroundDrawLists(window)->AddRect(borderRect.Min, borderRect.Max, IM_COL32(255, 255, 0, 255));
        if ((hovered && g.HoveredIdTimer > WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER) || held) {
            g.MouseCursor = (axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
            if (held)
                borderHeld = borderN;
        }
        if (held) {
            ImVec2 clamp_min(borderN == ImGuiDir_Right ? visibilityRect.Min.x : -FLT_MAX,
                             borderN == ImGuiDir_Down ? visibilityRect.Min.y : -FLT_MAX);
            ImVec2 clamp_max(borderN == ImGuiDir_Left ? visibilityRect.Max.x : +FLT_MAX,
                             borderN == ImGuiDir_Up ? visibilityRect.Max.y : +FLT_MAX);
            ImVec2 borderTarget = window->Pos;
            borderTarget[axis] = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + WINDOWS_HOVER_PADDING;
            borderTarget = ImClamp(borderTarget, clamp_min, clamp_max);
            calcResizePosSizeFromAnyCorner(window, borderTarget, ImMin(def.SegmentN1, def.SegmentN2), posTarget,
                                           sizeTarget);
        }
    }

    static void
    manualResizeGripItem(ImGuiWindow* window, ImGuiContext& g, const ImVec2& sizeAutoFit,
                         const float gripHoverInnerSize,
                         const float gripHoverOuterSize, const ImRect& visibilityRect,
                         std::array<ImU32, 4>& resizeGripCol,
                         int resizeGripN, ImVec2& posTarget, ImVec2& sizeTarget) {
        const ImGuiResizeGripDef& def = resize_grip_def[resizeGripN];

        const ImVec2 corner = ImLerp(window->Pos, {window->Pos.x + window->Size.x, window->Pos.y + window->Size.y},
                                     def.CornerPosN);

        // Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
        bool hovered;
        bool held;
        const ImVec2 min = {corner.x - def.InnerDir.x * gripHoverOuterSize,
                            corner.y - def.InnerDir.y * gripHoverOuterSize};
        const ImVec2 max = {corner.x + def.InnerDir.x * gripHoverOuterSize,
                            corner.y + def.InnerDir.y * gripHoverOuterSize};
        ImRect resizeRect(min, max);

        if (resizeRect.Min.x > resizeRect.Max.x) ImSwap(resizeRect.Min.x, resizeRect.Max.x);
        if (resizeRect.Min.y > resizeRect.Max.y) ImSwap(resizeRect.Min.y, resizeRect.Max.y);
        ImGuiID resize_grip_id = window->GetID(resizeGripN); // == GetWindowResizeCornerID()
        ImGui::ItemAdd(resizeRect, resize_grip_id);
        ImGui::ButtonBehavior(resizeRect, resize_grip_id, &hovered, &held,
                              ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
        //GetForegroundDrawList(window)->AddRect(resizeRect.Min, resizeRect.Max, IM_COL32(255, 255, 0, 255));
        if (hovered || held)
            g.MouseCursor = (resizeGripN & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE;

        if (held && g.IO.MouseDoubleClicked[0] && resizeGripN == 0) {
            // Manual auto-fit when double-clicking
            sizeTarget = calcWindowSizeAfterConstraint(window, sizeAutoFit);
            ImGui::ClearActiveID();
        } else if (held) {
            // Resize from any of the four corners
            // We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
            auto clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? visibilityRect.Min.x : -FLT_MAX,
                                    def.CornerPosN.y == 1.0f ? visibilityRect.Min.y : -FLT_MAX);
            auto clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? visibilityRect.Max.x : +FLT_MAX,
                                    def.CornerPosN.y == 0.0f ? visibilityRect.Max.y : +FLT_MAX);

            const float x = g.IO.MousePos.x - g.ActiveIdClickOffset.x +
                            ImLerp(def.InnerDir.x * gripHoverOuterSize, def.InnerDir.x * -gripHoverInnerSize,
                                   def.CornerPosN.x);
            const float y = g.IO.MousePos.y - g.ActiveIdClickOffset.y +
                            ImLerp(def.InnerDir.y * gripHoverOuterSize, def.InnerDir.y * -gripHoverInnerSize,
                                   def.CornerPosN.y);

            ImVec2 cornerTarget(x, y); // Corner of the window corresponding to our corner grip
            cornerTarget = ImClamp(cornerTarget, clamp_min, clamp_max);
            calcResizePosSizeFromAnyCorner(window, cornerTarget, def.CornerPosN, posTarget, sizeTarget);
        }

        // Only lower-left grip is visible before hovering/activating
        if (resizeGripN == 0 || held || hovered) {
            ImU32 value = ImGuiCol_ResizeGripActive;
            if (!held) {
                value = hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip;
            }
            resizeGripCol[resizeGripN] = ImGui::GetColorU32(value);
        }
    }

    static void manualResizeGrips(ImGuiWindow* window, ImGuiContext& g, const ImVec2& sizeAutoFit, const int resizeGripCount,
                                  const int resizeBorderCount, const float gripHoverInnerSize, const float gripHoverOuterSize,
                                  const ImRect& visibilityRect, ImVec2& posTarget, ImVec2& sizeTarget) {

        std::array<ImU32, 4> resizeGripCol = {};

        ImGui::PushID("#RESIZE");
        for (int resizeGripN = 0; resizeGripN < resizeGripCount; resizeGripN++) {
            manualResizeGripItem(window, g, sizeAutoFit, gripHoverInnerSize, gripHoverOuterSize, visibilityRect,
                                 resizeGripCol, resizeGripN, posTarget, sizeTarget);

        }

        int borderHeld = -1;
        for (int borderN = 0; borderN < resizeBorderCount; borderN++) {
            manualResizeGripHeld(window, g, gripHoverInnerSize, visibilityRect, posTarget,
                                 sizeTarget, borderN, borderHeld);

        }
        window->ResizeBorderHeld = (signed char) borderHeld;

        ImGui::PopID();
    }

// Exposed resize behavior for native OS windows
    bool UpdateWindowManualResize(ImGuiWindow* window, ImVec2& newSize, ImVec2& newPosition) {
        ENGINE_PROFILE_FUNCTION();

        ImGuiContext& g = *GImGui;

        // Decide if we are going to handle borders and resize grips

        if (!window->DockNodeAsHost || window->DockIsActive || window->Collapsed) {
            return false;
        }

        const ImVec2 sizeAutoFit = calcWindowAutoFitSize(window, window->ContentSizeIdeal);

        // Allow resize from lower-left if we have the mouse cursor feedback for it.
        const int resizeGripCount = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1;

        const auto& style = g.Style;

        if ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) ||
            window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0 || !window->WasActive) {
            return false;
        }

        const int resizeBorderCount = g.IO.ConfigWindowsResizeFromEdges ? 4 : 0;
        const auto gripDrawSize = IM_FLOOR(
                ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
        const auto gripHoverInnerSize = IM_FLOOR(gripDrawSize * 0.75f);
        const float gripHoverOuterSize = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;

        ImVec2 posTarget(FLT_MAX, FLT_MAX);
        ImVec2 sizeTarget(FLT_MAX, FLT_MAX);

        // Calculate the range of allowed position for that window (to be movable and visible past safe area padding)
        // When clamping to stay visible, we will enforce that window->Pos stays inside of visibilityRect.

        ImRect viewport_work_rect(window->Viewport->GetWorkRect());
        ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
        ImRect visibilityRect(
                {viewport_work_rect.Min.x + visibility_padding.x, viewport_work_rect.Min.y + visibility_padding.y},
                {viewport_work_rect.Max.x - visibility_padding.x, viewport_work_rect.Max.y - visibility_padding.y});

        // Clip mouse interaction rectangles within the viewport rectangle (in practice the narrowing is going to happen most of the time).
        // - Not narrowing would mostly benefit the situation where OS windows _without_ decoration have a threshold for hovering when outside their limits.
        //   This is however not the case with current backends under Win32, but a custom borderless window implementation would benefit from it.
        // - When decoration are enabled we typically benefit from that distance, but then our resize elements would be conflicting with OS resize elements, so we also narrow.
        // - Note that we are unable to tell if the platform setup allows hovering with a distance threshold (on Win32, decorated window have such threshold).
        // We only clip interaction so we overwrite window->ClipRect, cannot call PushClipRect() yet as DrawList is not yet setup.
        const bool clipWithViewportRect = !(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport) ||
                                          (g.IO.MouseHoveredViewport != window->ViewportId) ||
                                          !(window->Viewport->Flags & ImGuiViewportFlags_NoDecoration);
        if (clipWithViewportRect)
            window->ClipRect = window->Viewport->GetMainRect();

        // Resize grips and borders are on layer 1
        window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

        // Manual resize grips
        manualResizeGrips(window, g, sizeAutoFit, resizeGripCount, resizeBorderCount, gripHoverInnerSize,
                          gripHoverOuterSize, visibilityRect, posTarget, sizeTarget);

        bool changed = false;
        newSize = window->Size;
        newPosition = window->Pos;

        // Apply back modified position/size to window
        if (sizeTarget.x != FLT_MAX) {
            newSize = sizeTarget;
            changed = true;
        }
        if (posTarget.x != FLT_MAX) {
            newPosition = posTarget;
            changed = true;
        }

        return changed;
    }

    //=========================================================================================
    /// Shadows

    void DrawShadow(const Ref<Texture>& shadowImage, int radius, ImVec2 rectMin, ImVec2 rectMax, float alphMultiplier,
                    float lengthStretch,
                    bool drawLeft, bool drawRight, bool drawTop, bool drawBottom) {
        const float widthOffset = lengthStretch;
        const float alphaTop = std::min(0.25f * alphMultiplier, 1.0f);
        const float alphaSides = std::min(0.30f * alphMultiplier, 1.0f);
        const float alphaBottom = std::min(0.60f * alphMultiplier, 1.0f);
        const auto p1 = rectMin;
        const auto p2 = rectMax;

        auto* drawList = ImGui::GetWindowDrawList();
        if (drawLeft)
            drawList->AddImage(shadowImage->textureId(), {p1.x - widthOffset, p1.y - radius},
                               {p2.x + widthOffset, p1.y}, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                               ImColor(0.0f, 0.0f, 0.0f, alphaTop));
        if (drawRight)
            drawList->AddImage(shadowImage->textureId(), {p1.x - widthOffset, p2.y},
                               {p2.x + widthOffset, p2.y + radius}, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
                               ImColor(0.0f, 0.0f, 0.0f, alphaBottom));

        if (drawTop)
            drawList->AddImageQuad(shadowImage->textureId(), {p1.x - radius, p1.y - widthOffset},
                                   {p1.x, p1.y - widthOffset}, {p1.x, p2.y + widthOffset},
                                   {p1.x - radius, p2.y + widthOffset},
                                   {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
                                   ImColor(0.0f, 0.0f, 0.0f, alphaSides));
        if (drawBottom)
            drawList->AddImageQuad(shadowImage->textureId(), {p2.x, p1.y - widthOffset},
                                   {p2.x + radius, p1.y - widthOffset}, {p2.x + radius, p2.y + widthOffset},
                                   {p2.x, p2.y + widthOffset},
                                   {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
                                   ImColor(0.0f, 0.0f, 0.0f, alphaSides));
    };

    void
    DrawShadow(const Ref<Texture>& shadowImage, int radius, ImRect rectangle, float alphMultiplier, float lengthStretch,
               bool drawLeft, bool drawRight, bool drawTop, bool drawBottom) {
        DrawShadow(shadowImage, radius, rectangle.Min, rectangle.Max, alphMultiplier, lengthStretch, drawLeft,
                   drawRight, drawTop, drawBottom);
    };


    void DrawShadow(const Ref<Texture>& shadowImage, int radius, float alphMultiplier, float lengthStretch,
                    bool drawLeft, bool drawRight, bool drawTop, bool drawBottom) {
        DrawShadow(shadowImage, radius, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), alphMultiplier, lengthStretch,
                   drawLeft, drawRight, drawTop, drawBottom);
    };

    void DrawShadowInner(const Ref<Texture>& shadowImage, int radius, ImVec2 rectMin, ImVec2 rectMax, float alpha,
                         float lengthStretch,
                         bool drawLeft, bool drawRight, bool drawTop, bool drawBottom) {
        const float widthOffset = lengthStretch;
        const float alphaTop = alpha; //std::min(0.25f * alphMultiplier, 1.0f);
        const float alphaSides = alpha; //std::min(0.30f * alphMultiplier, 1.0f);
        const float alphaBottom = alpha; //std::min(0.60f * alphMultiplier, 1.0f);
        const auto p1 = ImVec2(rectMin.x + radius, rectMin.y + radius);
        const auto p2 = ImVec2(rectMax.x - radius, rectMax.y - radius);
        auto* drawList = ImGui::GetWindowDrawList();

        if (drawTop)
            drawList->AddImage(shadowImage->textureId(), {p1.x - widthOffset, p1.y - radius},
                               {p2.x + widthOffset, p1.y}, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
                               ImColor(0.0f, 0.0f, 0.0f, alphaTop));
        if (drawBottom)
            drawList->AddImage(shadowImage->textureId(), {p1.x - widthOffset, p2.y},
                               {p2.x + widthOffset, p2.y + radius}, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                               ImColor(0.0f, 0.0f, 0.0f, alphaBottom));
        if (drawLeft)
            drawList->AddImageQuad(shadowImage->textureId(), {p1.x - radius, p1.y - widthOffset},
                                   {p1.x, p1.y - widthOffset}, {p1.x, p2.y + widthOffset},
                                   {p1.x - radius, p2.y + widthOffset},
                                   {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
                                   ImColor(0.0f, 0.0f, 0.0f, alphaSides));
        if (drawRight)
            drawList->AddImageQuad(shadowImage->textureId(), {p2.x, p1.y - widthOffset},
                                   {p2.x + radius, p1.y - widthOffset}, {p2.x + radius, p2.y + widthOffset},
                                   {p2.x, p2.y + widthOffset},
                                   {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
                                   ImColor(0.0f, 0.0f, 0.0f, alphaSides));
    };

    void
    DrawShadowInner(const Ref<Texture>& shadowImage, int radius, ImRect rectangle, float alpha, float lengthStretch,
                    bool drawLeft, bool drawRight, bool drawTop, bool drawBottom) {
        DrawShadowInner(shadowImage, radius, rectangle.Min, rectangle.Max, alpha, lengthStretch, drawLeft, drawRight,
                        drawTop, drawBottom);
    };


    void DrawShadowInner(const Ref<Texture>& shadowImage, int radius, float alpha, float lengthStretch,
                         bool drawLeft, bool drawRight, bool drawTop, bool drawBottom) {
        DrawShadowInner(shadowImage, radius, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), alpha, lengthStretch,
                        drawLeft, drawRight, drawTop, drawBottom);
    }

    bool Toggle(const std::string_view& id, bool& toggleVal, ImGuiToggleFlags flags, ImVec2 size) {
        ScopedColour color(ImGuiCol_Button, ImVec4(0.16f, 0.66f, 0.45f, 1.0f));
        ScopedColour colorHover(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.57f, 1.0f));

        return ImGui::Toggle(id.data(), &toggleVal, flags, size);
    }

    bool InputText(const std::string_view& id, std::string& value, ImGuiInputTextFlags flags) {
        return ImGui::InputText(id.data(), &value, flags);
    }

    std::filesystem::path getUniquePath(int& counter, const std::filesystem::path& fp) {
        counter++;

        const std::string counterStr = counter < 10 ? myFormat("0{}", counter) : std::to_string(counter);

        const std::filesystem::path basePath = myFormat("{}_{}{}", Utils::RemoveExtension(fp.string()), counterStr, fp.extension().string());
        if (std::filesystem::exists(basePath)) {
            return getUniquePath(counter, fp);
        } else {
            return basePath;
        }
    }

    std::filesystem::path GetUniquePath(const std::filesystem::path& fp) {
        int counter = 0;
        return getUniquePath(counter, fp);
    }

    void PopupCloseButton() {
        ImGui::Spacing();

        const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
        auto btnStr = myFormat(ICON_FA_XMARK " {}", I18N::Get("CLOSE"));
        const float buttonWidth = ImGui::CalcTextSize(btnStr.c_str()).x + 36;

        UI::Core::ShiftCursorX(((contentRegionWidth - buttonWidth) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

        if (ImGui::Button(btnStr.c_str(), ImVec2(buttonWidth, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }
    }
}
