//
// Created by pierr on 16/03/2023.
//
#pragma once

#include <string>
#include "ViewerApp/CoreLib/Events/AEvent.h"
#include "imgui.h"

namespace Astra::UI::Core {

    class AWindow
    {
    public:
        const std::string title;
        static constexpr int DEFAULT_WIDTH = 1200;
        static constexpr int DEFAULT_HEIGHT = 800;

        explicit AWindow(std::string pTitle)
                : title(std::move(pTitle)) {}

        virtual ~AWindow() = default;
        AWindow(const AWindow&) = delete;
        AWindow& operator=(const AWindow&) = delete;

        virtual void initWindow() = 0;

        virtual void processEvents() const = 0;

        virtual bool isWindowMaximized() const = 0;
        virtual void maximizeWindow() const = 0;
        virtual void restoreWindow() const = 0;
        virtual void iconifyWindow() const = 0;
        virtual void getWindowPos(int& pPosX, int& pPosY) const = 0;
        virtual void getWindowSize(int& pSizeX, int& pSizeY) const = 0;
        virtual void setWindowPos(int pPosX, int pPosY) const = 0;
        virtual void setWindowSize(int pSizeX, int pSizeY) const = 0;

        virtual void initImGui() = 0;
        virtual void shutdownImGui() = 0;
        virtual void newFrameImGui() = 0;
        virtual void endFrameImGui() = 0;
        virtual void viewportImGui() = 0;

    };


}
