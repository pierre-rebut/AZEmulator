//
// Created by pierr on 15/03/2023.
//
#pragma once

#include "ViewerApp/Application.h"

#include "CpuEngine/CpuContext.h"

#include "ViewerApp/CoreLib/CoreEngine.h"

namespace Astra::UI::App {

    class ViewerApplication : public Application
    {
    private:
        CPU::Core::CpuContext m_cpuContext;
        Scope<UI::Core::CoreEngine> m_imGuiEngine;

    public:
        explicit ViewerApplication();
        ~ViewerApplication() override;

        void run() override;
    };

}
