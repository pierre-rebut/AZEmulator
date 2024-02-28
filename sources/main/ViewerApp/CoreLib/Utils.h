//
// Created by pierr on 21/03/2023.
//
#pragma once

#include <cstring>
#include <algorithm>
#include <string_view>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "Colors.h"
#include "ViewerApp/CoreLib/Resources/Texture.h"
#include "EngineLib/data/Base.h"
#include "imgui_toggle/imgui_toggle.h"
#include "Commons/Log.h"

namespace Astra::UI::Core {

    bool Toggle(const std::string_view& id, bool& toggleVal, ImGuiToggleFlags flags = ImGuiToggleFlags_Animated, ImVec2 size = ImVec2(35, 20));

    bool InputText(const std::string_view& id, std::string& value, ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank);
    std::filesystem::path getUniquePath(int& counter, const std::filesystem::path& fp);
    std::filesystem::path GetUniquePath(const std::filesystem::path& fp);

    void PopupCloseButton();

    /// Utilities
    class ScopedStyle
    {
    public:
        ScopedStyle(const ScopedStyle&) = delete;
        ScopedStyle& operator=(const ScopedStyle&) = delete;

        template<typename T>
        ScopedStyle(ImGuiStyleVar styleVar, T value) { ImGui::PushStyleVar(styleVar, value); }

        ~ScopedStyle() { ImGui::PopStyleVar(); }
    };

    class ScopedColour
    {
    public:
        ScopedColour(const ScopedColour&) = delete;
        ScopedColour& operator=(const ScopedColour&) = delete;

        template<typename T>
        ScopedColour(ImGuiCol colourId, T colour) { ImGui::PushStyleColor(colourId, colour); }

        ~ScopedColour() { ImGui::PopStyleColor(); }
    };

    class ScopedFont
    {
    public:
        ScopedFont(const ScopedFont&) = delete;
        ScopedFont& operator=(const ScopedFont&) = delete;

        explicit ScopedFont(ImFont* font) { ImGui::PushFont(font); }

        ~ScopedFont() { ImGui::PopFont(); }
    };

    class ScopedColourStack
    {
    public:
        ScopedColourStack(const ScopedColourStack&) = delete;
        ScopedColourStack& operator=(const ScopedColourStack&) = delete;

        template<typename ColourType, typename... OtherColours>
        ScopedColourStack(ImGuiCol firstColourID, ColourType firstColour, OtherColours&& ... otherColourPairs)
                : m_Count((sizeof... (otherColourPairs) / 2) + 1) {
            static_assert((sizeof... (otherColourPairs) & 1u) == 0,
                          "ScopedColourStack constructor expects a list of pairs of colour IDs and colours as its arguments");

            PushColour(firstColourID, firstColour, std::forward<OtherColours>(otherColourPairs)...);
        }

        ~ScopedColourStack() { ImGui::PopStyleColor(m_Count); }

    private:
        int m_Count;

        template<typename ColourType, typename... OtherColours>
        void PushColour(ImGuiCol colourID, ColourType colour, OtherColours&& ... otherColourPairs) {
            if constexpr (sizeof... (otherColourPairs) == 0) {
                ImGui::PushStyleColor(colourID, colour);
            } else {
                ImGui::PushStyleColor(colourID, colour);
                PushColour(std::forward<OtherColours>(otherColourPairs)...);
            }
        }
    };

    class ScopedStyleStack
    {
    public:
        ScopedStyleStack(const ScopedStyleStack&) = delete;
        ScopedStyleStack& operator=(const ScopedStyleStack&) = delete;

        template<typename ValueType, typename... OtherStylePairs>
        ScopedStyleStack(ImGuiStyleVar firstStyleVar, ValueType firstValue, OtherStylePairs&& ... otherStylePairs)
                : m_Count((sizeof... (otherStylePairs) / 2) + 1) {
            static_assert((sizeof... (otherStylePairs) & 1u) == 0,
                          "ScopedStyleStack constructor expects a list of pairs of colour IDs and colours as its arguments");

            PushStyle(firstStyleVar, firstValue, std::forward<OtherStylePairs>(otherStylePairs)...);
        }

        ~ScopedStyleStack() { ImGui::PopStyleVar(m_Count); }

    private:
        int m_Count;

        template<typename ValueType, typename... OtherStylePairs>
        void PushStyle(ImGuiStyleVar styleVar, ValueType value, OtherStylePairs&& ... otherStylePairs) {
            if constexpr (sizeof... (otherStylePairs) == 0) {
                ImGui::PushStyleVar(styleVar, value);
            } else {
                ImGui::PushStyleVar(styleVar, value);
                PushStyle(std::forward<OtherStylePairs>(otherStylePairs)...);
            }
        }
    };

    /// Colours

    static ImU32 ColourWithValue(const ImColor& color, float value) {
        const ImVec4& colRow = color.Value;
        float hue, sat, val;
        ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
        return ImColor::HSV(hue, sat, std::min(value, 1.0f));
    }

    static ImU32 ColourWithSaturation(const ImColor& color, float saturation) {
        const ImVec4& colRow = color.Value;
        float hue, sat, val;
        ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
        return ImColor::HSV(hue, std::min(saturation, 1.0f), val);
    }

    static ImU32 ColourWithHue(const ImColor& color, float hue) {
        const ImVec4& colRow = color.Value;
        float h, s, v;
        ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, h, s, v);
        return ImColor::HSV(std::min(hue, 1.0f), s, v);
    }

    static ImU32 ColourWithMultipliedValue(const ImColor& color, float multiplier) {
        const ImVec4& colRow = color.Value;
        float hue, sat, val;
        ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
        return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
    }

    static ImU32 ColourWithMultipliedSaturation(const ImColor& color, float multiplier) {
        const ImVec4& colRow = color.Value;
        float hue, sat, val;
        ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
        return ImColor::HSV(hue, std::min(sat * multiplier, 1.0f), val);
    }

    static ImU32 ColourWithMultipliedHue(const ImColor& color, float multiplier) {
        const ImVec4& colRow = color.Value;
        float hue, sat, val;
        ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
        return ImColor::HSV(std::min(hue * multiplier, 1.0f), sat, val);
    }

    /// Button Image

    static void DrawButtonImage(const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
                                ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
                                ImVec2 rectMin, ImVec2 rectMax) {
        auto* drawList = ImGui::GetWindowDrawList();
        if (ImGui::IsItemActive())
            drawList->AddImage(imagePressed->textureId(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
        else if (ImGui::IsItemHovered())
            drawList->AddImage(imageHovered->textureId(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
        else
            drawList->AddImage(imageNormal->textureId(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
    };

    static void DrawButtonImage(const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
                                ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
                                ImRect rectangle) {
        DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
    };

    static void DrawButtonImage(const Ref<Texture>& image,
                                ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
                                ImVec2 rectMin, ImVec2 rectMax) {
        DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
    };

    static void DrawButtonImage(const Ref<Texture>& image,
                                ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
                                ImRect rectangle) {
        DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
    };


    static void DrawButtonImage(const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
                                ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed) {
        DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    };

    static void DrawButtonImage(const Ref<Texture>& image,
                                ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed) {
        DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }

    //=========================================================================================

    static void DrawButtonTextImage(const char* text, const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
                                    ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
                                    ImVec2 rectMin, ImVec2 rectMax) {
        auto* drawList = ImGui::GetWindowDrawList();
        if (ImGui::IsItemActive())
            drawList->AddImage(imagePressed->textureId(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
        else if (ImGui::IsItemHovered())
            drawList->AddImage(imageHovered->textureId(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
        else
            drawList->AddImage(imageNormal->textureId(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);

        const ImVec2 label_size = ImGui::CalcTextSize(text);
        const ImVec2 pos(
                ((rectMax.x - rectMin.x) / 2) - 5 - (label_size.x / 2) + rectMin.x,
                ((rectMax.y - rectMin.y) / 2) - (label_size.y / 2) + rectMin.y
        );

        drawList->AddText(pos, IM_COL32(255, 255, 255, 255), text);
    };

    static void DrawButtonTextImage(const char* text, const Ref<Texture>& image,
                                    ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
                                    ImRect rectangle) {
        DrawButtonTextImage(text, image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
    };


    //=========================================================================================

    /// Border
    static void DrawBorder(ImVec2 rectMin, ImVec2 rectMax, const ImVec4& borderColour, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        auto min = rectMin;
        min.x -= thickness;
        min.y -= thickness;
        min.x += offsetX;
        min.y += offsetY;
        auto max = rectMax;
        max.x += thickness;
        max.y += thickness;
        max.x += offsetX;
        max.y += offsetY;

        auto* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(borderColour), 0.0f, 0, thickness);
    };

    static void DrawBorder(const ImVec4& borderColour, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorder(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), borderColour, thickness, offsetX, offsetY);
    };

    static void DrawBorder(float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorder(ImGui::GetStyleColorVec4(ImGuiCol_Border), thickness, offsetX, offsetY);
    };

    static void DrawBorder(ImVec2 rectMin, ImVec2 rectMax, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorder(rectMin, rectMax, ImGui::GetStyleColorVec4(ImGuiCol_Border), thickness, offsetX, offsetY);
    };

    static void DrawBorder(ImRect rect, float thickness = 1.0f, float rounding = 0.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        auto min = rect.Min;
        min.x -= thickness;
        min.y -= thickness;
        min.x += offsetX;
        min.y += offsetY;
        auto max = rect.Max;
        max.x += thickness;
        max.y += thickness;
        max.x += offsetX;
        max.y += offsetY;

        auto* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)), rounding, 0, thickness);
    };

    static void
    DrawBorderHorizontal(ImVec2 rectMin, ImVec2 rectMax, const ImVec4& borderColour, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        auto min = rectMin;
        min.y -= thickness;
        min.x += offsetX;
        min.y += offsetY;
        auto max = rectMax;
        max.y += thickness;
        max.x += offsetX;
        max.y += offsetY;

        auto* drawList = ImGui::GetWindowDrawList();
        const auto colour = ImGui::ColorConvertFloat4ToU32(borderColour);
        drawList->AddLine(min, ImVec2(max.x, min.y), colour, thickness);
        drawList->AddLine(ImVec2(min.x, max.y), max, colour, thickness);
    };

    static void DrawBorderHorizontal(ImVec2 rectMin, ImVec2 rectMax, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorderHorizontal(rectMin, rectMax, ImGui::GetStyleColorVec4(ImGuiCol_Border), thickness, offsetX, offsetY);
    };

    static void DrawBorderHorizontal(const ImVec4& borderColour, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorderHorizontal(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), borderColour, thickness, offsetX, offsetY);
    };

    static void DrawBorderHorizontal(float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorderHorizontal(ImGui::GetStyleColorVec4(ImGuiCol_Border), thickness, offsetX, offsetY);
    };

    static void
    DrawBorderVertical(ImVec2 rectMin, ImVec2 rectMax, const ImVec4& borderColour, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        auto min = rectMin;
        min.x -= thickness;
        min.x += offsetX;
        min.y += offsetY;
        auto max = rectMax;
        max.x += thickness;
        max.x += offsetX;
        max.y += offsetY;

        auto* drawList = ImGui::GetWindowDrawList();
        const auto colour = ImGui::ColorConvertFloat4ToU32(borderColour);
        drawList->AddLine(min, ImVec2(min.x, max.y), colour, thickness);
        drawList->AddLine(ImVec2(max.x, min.y), max, colour, thickness);
    };

    static void DrawBorderVertical(const ImVec4& borderColour, float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorderVertical(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), borderColour, thickness, offsetX, offsetY);
    };

    static void DrawBorderVertical(float thickness = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f) {
        DrawBorderVertical(ImGui::GetStyleColorVec4(ImGuiCol_Border), thickness, offsetX, offsetY);
    };

    //=========================================================================================
    /// Shadows

    void DrawShadow(const Ref<Texture>& shadowImage, int radius, ImVec2 rectMin, ImVec2 rectMax, float alphMultiplier = 1.0f, float lengthStretch = 10.0f,
                    bool drawLeft = true, bool drawRight = true, bool drawTop = true, bool drawBottom = true);

    void DrawShadow(const Ref<Texture>& shadowImage, int radius, ImRect rectangle, float alphMultiplier = 1.0f, float lengthStretch = 10.0f,
                    bool drawLeft = true, bool drawRight = true, bool drawTop = true, bool drawBottom = true);


    void DrawShadow(const Ref<Texture>& shadowImage, int radius, float alphMultiplier = 1.0f, float lengthStretch = 10.0f,
                    bool drawLeft = true, bool drawRight = true, bool drawTop = true, bool drawBottom = true);

    void DrawShadowInner(const Ref<Texture>& shadowImage, int radius, ImVec2 rectMin, ImVec2 rectMax, float alpha = 1.0f, float lengthStretch = 10.0f,
                         bool drawLeft = true, bool drawRight = true, bool drawTop = true, bool drawBottom = true);

    void DrawShadowInner(const Ref<Texture>& shadowImage, int radius, ImRect rectangle, float alpha = 1.0f, float lengthStretch = 10.0f,
                         bool drawLeft = true, bool drawRight = true, bool drawTop = true, bool drawBottom = true);


    void DrawShadowInner(const Ref<Texture>& shadowImage, int radius, float alpha = 1.0f, float lengthStretch = 10.0f,
                         bool drawLeft = true, bool drawRight = true, bool drawTop = true, bool drawBottom = true);

    //=========================================================================================
    /// Cursor

    static void ShiftCursorX(float distance) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
    }

    static void ShiftCursorY(float distance) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
    }

    static void ShiftCursor(float x, float y) {
        const ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cursor.x + x, cursor.y + y));
    }

    //=========================================================================================
    /// Rectangle

    static inline ImRect GetItemRect() {
        return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }

    static inline ImRect RectExpanded(const ImRect& rect, float x, float y) {
        ImRect result = rect;
        result.Min.x -= x;
        result.Min.y -= y;
        result.Max.x += x;
        result.Max.y += y;
        return result;
    }

    static inline ImRect RectOffset(const ImRect& rect, float x, float y) {
        ImRect result = rect;
        result.Min.x += x;
        result.Min.y += y;
        result.Max.x += x;
        result.Max.y += y;
        return result;
    }

    static inline ImRect RectOffset(const ImRect& rect, ImVec2 xy) {
        return RectOffset(rect, xy.x, xy.y);
    }

    //==========================================================================
    static bool IsItemHovered(float delayInSeconds = 0.1f, ImGuiHoveredFlags flags = 0) {
        return ImGui::IsItemHovered(flags) && GImGui->HoveredIdTimer > delayInSeconds; /*HoveredIdNotActiveTimer*/
    }

    static void SetTooltip(const std::string_view& text, float delayInSeconds = 0.1f, bool allowWhenDisabled = true, ImVec2 padding = ImVec2(5, 5)) {
        if (IsItemHovered(delayInSeconds, allowWhenDisabled ? ImGuiHoveredFlags_AllowWhenDisabled : 0)) {
            UI::Core::ScopedStyle tooltipPadding(ImGuiStyleVar_WindowPadding, padding);
            UI::Core::ScopedColour textCol(ImGuiCol_Text, UI::Core::Colors::textBrighter);
            ImGui::SetTooltip(text.data());
        }
    }

    static void SetTooltipMultiLine(const std::vector<std::string>& texts, ImVec2 padding = ImVec2(5, 5)) {
        UI::Core::ScopedStyle tooltipPadding(ImGuiStyleVar_WindowPadding, padding);
        UI::Core::ScopedColour textCol(ImGuiCol_Text, UI::Core::Colors::textBrighter);

        if (!ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
            return;

        for (const auto& text: texts) {
            ImGui::Text(text.data());
        }

        ImGui::EndTooltip();
    }

    static void DrawItemActivityOutline(float rounding = 0.0f, bool drawWhenInactive = false, ImColor colourWhenActive = ImColor(80, 80, 80)) {
        auto* drawList = ImGui::GetWindowDrawList();
        const ImRect rect = RectExpanded(GetItemRect(), 1.0f, 1.0f);
        if (ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
            drawList->AddRect(rect.Min, rect.Max,
                              ImColor(60, 60, 60), rounding, 0, 1.5f);
        }
        if (ImGui::IsItemActive()) {
            drawList->AddRect(rect.Min, rect.Max,
                              colourWhenActive, rounding, 0, 1.0f);
        } else if (!ImGui::IsItemHovered() && drawWhenInactive) {
            drawList->AddRect(rect.Min, rect.Max,
                              ImColor(50, 50, 50), rounding, 0, 1.0f);
        }
    };

    //=========================================================================================
    /// Window

    bool BeginPopup(const char* strId, ImGuiWindowFlags flags = 0);
    void EndPopup();

    // MenuBar which allows you to specify its rectangle
    bool BeginMenuBar(const ImRect& barRectangle);
    void EndMenuBar();

    // Exposed to be used for window with disabled decorations
    // This border is going to be drawn even if window border size is set to 0.0f
    void RenderWindowOuterBorders(ImGuiWindow* window);

    bool UpdateWindowManualResize(ImGuiWindow* window, ImVec2& sizeContents, ImVec2& newPosition);
}
