//
// Created by pierr on 14/07/2023.
//

#include "ImGuiUtils.h"
#include "imgui.h"

namespace Astra::UI::Core {
    void ImGuiUtils::forceColumnMaxSize(int nbColumn, int row, int minRow) {
        if (row == minRow) {
            for (int i = 0; i < nbColumn; i++) {
                ImGui::TableSetColumnIndex(i);
                ImGui::PushItemWidth(-FLT_MIN);
            }
        }
    }
}
