//
// Created by pierr on 28/03/2023.
//
#pragma once

#include <algorithm>
#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::Core {
    class Widgets
    {
    public:

        template<uint32_t BuffSize = 256, typename StringType>
        static bool SearchWidget(const char* id, StringType& searchString, const char* hint = "Search...",
                                 bool* grabFocus = nullptr) {
            ImGui::PushID(id);

            bool modified = false;
            bool searching;

            const float areaPosX = ImGui::GetCursorPosX();
            const float framePaddingY = ImGui::GetStyle().FramePadding.y;

            UI::Core::ScopedStyle rounding(ImGuiStyleVar_FrameRounding, 3.0f);
            UI::Core::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(28.0f, framePaddingY));

            if constexpr (std::is_same_v<StringType, std::string>) {
                char searchBuffer[BuffSize]{};
                strcpy_s<BuffSize>(searchBuffer, searchString.c_str());
                if (ImGui::InputText("##search", searchBuffer, BuffSize) || ImGui::IsItemDeactivatedAfterEdit()) {
                    searchString = searchBuffer;
                    modified = true;
                }

                searching = searchBuffer[0] != 0;
            } else if constexpr (std::is_same_v<StringType, unsigned int>) {
                if (ImGui::InputScalar("##search", ImGuiDataType_U32, &searchString, nullptr, nullptr, "%08X",
                                       ImGuiInputTextFlags_CharsHexadecimal |
                                       ImGuiInputTextFlags_AutoSelectAll)) {
                    modified = true;
                } else if (ImGui::IsItemDeactivatedAfterEdit()) {
                    modified = true;
                }

                searching = true;
            } else {
                static_assert(std::is_same_v<decltype(&searchString[0]), char*>,
                              "searchString paramenter must be std::string& or char*");

                if (ImGui::InputText("##search", searchString, BuffSize)) {
                    modified = true;
                } else if (ImGui::IsItemDeactivatedAfterEdit()) {
                    modified = true;
                }

                searching = searchString[0] != 0;
            }

            if (grabFocus && *grabFocus) {
                if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
                    && !ImGui::IsAnyItemActive()
                    && !ImGui::IsMouseClicked(0)) {
                    ImGui::SetKeyboardFocusHere(-1);
                }

                if (ImGui::IsItemFocused())
                    *grabFocus = false;
            }

            UI::Core::DrawItemActivityOutline(3.0f, true, Colors::accent);
            ImGui::SetItemAllowOverlap();

            ImGui::SameLine(areaPosX + 5.0f);

            displaySearchIcon(hint, searching, framePaddingY);

            ImGui::PopID();
            return modified;
        }

        static void
        displaySearchIcon(const char* hint, bool searching, const float framePaddingY) {

            ImGui::Text(ICON_FA_MAGNIFYING_GLASS);

            // Hint
            if (!searching) {
                ImGui::SameLine(0, 10);
                ScopedColour text(ImGuiCol_Text, Colors::textDarker);
                ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, framePaddingY));
                ImGui::TextUnformatted(hint);
            }
        }

        static bool IconButton(const char *txt) {
            auto pos = ImGui::GetCursorScreenPos();
            const bool clicked = ImGui::InvisibleButton(txt, ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()});
            if (ImGui::IsItemHovered()) {
                ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 8.5f, pos.y + 3.8f), IM_COL32(255, 255, 255, 255), txt);
            } else {
                ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 8.5f, pos.y + 3.8f), Colors::text, txt);
            }
            return clicked;
        }
    };
}
