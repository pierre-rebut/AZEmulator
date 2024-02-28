//
// Created by pierr on 14/07/2023.
//

#pragma once

namespace Astra::UI::Core {
    class ImGuiUtils
    {
    public:
        ImGuiUtils() = delete;

        static void forceColumnMaxSize(int nbColumn, int row, int minRow);

    };
}
