//
// Created by pierr on 16/03/2023.
//

#include "CustomTreeNode.h"
#include "ViewerApp/CoreLib/Utils.h"


namespace Astra::UI::Core {

    static bool isToggled(ImGuiID id, ImGuiTreeNodeFlags flags, const ImGuiContext& g, bool isOpen,
                          const bool isMouseXOverArrow, bool pressed) {
        bool toggled = false;

        if (pressed && g.DragDropHoldJustPressedId != id) {
            if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 ||
                (g.NavActivateId == id)) {
                toggled = true;
            }
            if (flags & ImGuiTreeNodeFlags_OpenOnArrow) {
                toggled |= isMouseXOverArrow &&
                           !g.NavDisableMouseHover;
            } // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
            if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
                toggled = true;
        } else if (pressed && g.DragDropHoldJustPressedId == id && !isOpen) {
            // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
            toggled = true;
        }

        if (g.NavId == id && g.NavMoveDir == ImGuiDir_Left && isOpen) {
            toggled = true;
            ImGui::NavMoveRequestCancel();
        }
        if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right &&
            !isOpen) // If there's something upcoming on the line we may want to give it the priority?
        {
            toggled = true;
            ImGui::NavMoveRequestCancel();
        }
        return toggled;
    }

    bool TreeNode(const std::string& id, const std::string& label, ImGuiTreeNodeFlags flags, const Ref<UI::Core::Texture>& icon) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        return TreeNodeWithIcon(icon, window->GetID(id.c_str()), flags, label.c_str(), nullptr);
    }

    bool TreeNodeWithIcon(const Ref<Texture>& icon, ImGuiID id, ImGuiTreeNodeFlags flags, const char* label,
                          const char* label_end, ImColor iconTint) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        ImGuiContext& g = *GImGui;
        ImGuiLastItemData& lastItem = g.LastItemData;
        const ImGuiStyle& style = g.Style;
        const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
        const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding
                                                                                            : ImVec2(
                        style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

        if (!label_end) {
            label_end = ImGui::FindRenderedTextEnd(label);
        }
        const ImVec2 label_size = ImGui::CalcTextSize(label, label_end, false);

        // We vertically grow up to current line height up the typical widget height.
        const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2),
                                         label_size.y + padding.y * 2);
        ImRect frame_bb;
        frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
        frame_bb.Min.y = window->DC.CursorPos.y;
        frame_bb.Max.x = window->WorkRect.Max.x;
        frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
        if (display_frame) {
            // Framed header expand a little outside the default padding, to the edge of InnerClipRect
            // (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
            frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
            frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
        }

        // Collapser arrow width + Spacing
        const float text_offset_x = g.FontSize + (
                display_frame ? padding.x * 3 : padding.x * 2
        );

        // Latch before ItemSize changes it
        const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);

        // Include collapser
        const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);

        ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
        ImGui::ItemSize(ImVec2(text_width, frame_height), padding.y);

        // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
        ImRect interact_bb = frame_bb;
        if (!display_frame && (flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0) {
            interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;
        }

        // Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
        // For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
        // This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
        const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
        bool isOpen = ImGui::TreeNodeBehaviorIsOpen(id, flags);
        if (isOpen && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) &&
            !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
            window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);
        }

        bool item_add = ImGui::ItemAdd(interact_bb, id);
        lastItem.StatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
        lastItem.DisplayRect = frame_bb;

        if (!item_add) {
            if (isOpen && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                ImGui::TreePushOverrideID(id);
            }
            IMGUI_TEST_ENGINE_ITEM_INFO(lastItem.ID, label,
                                        lastItem.StatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) |
                                        (isOpen ? ImGuiItemStatusFlags_Opened : 0));
            return isOpen;
        }

        ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
        if (flags & ImGuiTreeNodeFlags_AllowItemOverlap) {
            button_flags |= ImGuiButtonFlags_AllowOverlap;
        }
        if (!is_leaf) {
            button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;
        }

        // We allow clicking on the arrow section with keyboard modifiers held, in order to easily
        // allow browsing a tree while preserving selection with code implementing multi-selection patterns.
        // When clicking on the rest of the tree node we always disallow keyboard modifiers.
        const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
        const float arrow_hit_x2 =
                (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
        const bool isMouseXOverArrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
        if (window != g.HoveredWindow || !isMouseXOverArrow) {
            button_flags |= ImGuiButtonFlags_NoKeyModifiers;
        }

        // Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
        // Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
        // - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
        // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
        // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
        // - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
        // - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
        // It is rather standard that arrow click react on Down rather than Up.
        // We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
        if (isMouseXOverArrow) {
            button_flags |= ImGuiButtonFlags_PressedOnClick;
        } else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) {
            button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
        } else {
            button_flags |= ImGuiButtonFlags_PressedOnClickRelease;
        }

        bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
        const bool was_selected = selected;

        bool hovered;
        bool held;
        bool isPressed = ImGui::ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
        bool toggled = false;

        if (!is_leaf) {
            toggled = isToggled(id, flags, g, isOpen, isMouseXOverArrow, isPressed);

            if (toggled) {
                isOpen = !isOpen;
                window->DC.StateStorage->SetInt(id, isOpen);
                lastItem.StatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
            }
        }
        if (flags & ImGuiTreeNodeFlags_AllowItemOverlap) {
            ImGui::SetItemAllowOverlap();
        }

        // In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
        if (selected != was_selected) { //-V547
            lastItem.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;
        }

        // Render
        const ImU32 arrow_col = selected ? Colors::backgroundDark : Colors::muted;

        ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
        if (display_frame) {
            // Framed type
            ImGuiCol color = ImGuiCol_HeaderActive;
            if (!(held && hovered)) {
                color = (hovered && !selected && !held && !isPressed && !toggled)
                        ? ImGuiCol_HeaderHovered : ImGuiCol_Header;
            }
            const ImU32 bg_col = ImGui::GetColorU32(color);

            ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
            ImGui::RenderNavHighlight(frame_bb, id, nav_highlight_flags);
            if (flags & ImGuiTreeNodeFlags_Bullet) {
                ImGui::RenderBullet(window->DrawList,
                                    ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f),
                                    arrow_col);
            } else if (!is_leaf) {
                ImGui::RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y),
                                   arrow_col, isOpen ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
            } else { // Leaf without bullet, left-adjusted text
                text_pos.x -= text_offset_x;
            }
            if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton) {
                frame_bb.Max.x -= g.FontSize + style.FramePadding.x;
            }

            //! Draw icon
            if (icon) {
                // Store item data
                auto itemId = lastItem.ID;
                auto itemFlags = lastItem.InFlags;
                auto itemStatusFlags = lastItem.StatusFlags;
                auto itemRect = lastItem.Rect;

                // Draw icon image which messes up last item data
                const float pad = 3.0f;
                const float arrowWidth = 20.0f + 1.0f;

                ShiftCursorY(-frame_height + pad);
                ShiftCursorX(arrowWidth);
                ImGui::Image(icon->textureId(), {frame_height - pad * 2.0f, frame_height - pad * 2.0f}, ImVec2(0, 0),
                             ImVec2(1, 1), iconTint /*selected ? colourDark : tintFloat*/);

                // Restore itme data
                ImGui::SetLastItemData(itemId, itemFlags, itemStatusFlags, itemRect);

                text_pos.x += frame_height + 2.0f;
            }

            text_pos.y -= 1.0f;


            if (g.LogEnabled) {
                // NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
                const char log_prefix[] = "\n##";
                const char log_suffix[] = "##";
                ImGui::LogRenderedText(&text_pos, log_prefix, log_prefix + 3);
                ImGui::RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
                ImGui::LogRenderedText(&text_pos, log_suffix, log_suffix + 2);
            } else {
                ImGui::RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
            }
        } else {
            // Unframed typed for tree nodes
            if (hovered || selected) {

                ImGuiCol color = ImGuiCol_HeaderActive;
                if (!(held && hovered)) {
                    color = (hovered &&
                             !selected &&
                             !held &&
                             !isPressed &&
                             !toggled
                            )
                            ? ImGuiCol_HeaderHovered
                            : ImGuiCol_Header;
                }

                const ImU32 bg_col = ImGui::GetColorU32(color);
                ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
                ImGui::RenderNavHighlight(frame_bb, id, nav_highlight_flags);
            }
            if (flags & ImGuiTreeNodeFlags_Bullet) {
                ImGui::RenderBullet(window->DrawList,
                                    ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f),
                                    arrow_col);
            } else if (!is_leaf) {
                ImGui::RenderArrow(window->DrawList,
                                   ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f),
                                   arrow_col, isOpen ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
            }

            //! Draw icon
            if (icon) {
                // Store item data
                auto itemId = lastItem.ID;
                auto itemFlags = lastItem.InFlags;
                auto itemStatusFlags = lastItem.StatusFlags;
                auto itemRect = lastItem.Rect;

                // Draw icon image which messes up last item data
                const float pad = 3.0f;
                const float arrowWidth = 20.0f + 1.0f;

                ShiftCursorY(-frame_height + pad);
                ShiftCursorX(arrowWidth);
                ImGui::Image(icon->textureId(), {frame_height - pad * 2.0f, frame_height - pad * 2.0f}, ImVec2(0, 0),
                             ImVec2(1, 1), iconTint /*selected ? colourDark : tintFloat*/);

                // Restore itme data
                ImGui::SetLastItemData(itemId, itemFlags, itemStatusFlags, itemRect);

                text_pos.x += frame_height + 2.0f;
            }

            text_pos.y -= 1.0f;


            if (g.LogEnabled) {
                ImGui::LogRenderedText(&text_pos, ">");
            }
            ImGui::RenderText(text_pos, label, label_end, false);
        }

        if (isOpen && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
            ImGui::TreePushOverrideID(id);
        }
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) |
                                               (isOpen ? ImGuiItemStatusFlags_Opened : 0));
        return isOpen;
    }

    bool
    TreeNodeWithIcon(const Ref<Texture>& icon, const char* label, ImGuiTreeNodeFlags flags,
                     ImColor iconTint /*= IM_COL32_WHITE*/) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        return TreeNodeWithIcon(icon, window->GetID(label), flags, label, nullptr, iconTint);
    }

}
