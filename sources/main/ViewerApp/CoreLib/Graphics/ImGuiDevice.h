//
// Created by pierr on 15/03/2023.
//
#pragma once

#include <cstdint>
#include <string>

#include "AWindow.h"
#include "EngineLib/data/Base.h"

namespace Astra::UI::Core {

    class ImGuiDevice
    {
    private:
        AWindow* m_window;

    public:
        explicit ImGuiDevice(AWindow* pWindow);

        ~ImGuiDevice();

        void NewFrame();

        void EndFrame() const;
    };

}
