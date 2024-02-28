//
// Created by pierr on 16/03/2023.
//
#pragma once

#include <vector>
#include <string>

#include "ViewerApp/CoreLib/Graphics/AWindow.h"

#include <GLFW/glfw3.h>

namespace Astra::UI::Core::Glfw {

    class GlfwWindow : public AWindow
    {
    public:
        explicit GlfwWindow(std::string pWindowName);
        ~GlfwWindow() override;
        GlfwWindow(const GlfwWindow&) = delete;
        GlfwWindow& operator=(const GlfwWindow&) = delete;

        void initWindow() override;

        void processEvents() const override { glfwPollEvents(); }

        bool isWindowMaximized() const override;
        void maximizeWindow() const override;
        void restoreWindow() const override;
        void iconifyWindow() const override;
        void getWindowPos(int& pPosX, int& pPosY) const override;
        void getWindowSize(int& pSizeX, int& pSizeY) const override;
        void setWindowPos(int pPosX, int pPosY) const override;
        void setWindowSize(int pSizeX, int pSizeY) const override;

        void initImGui() override;
        void shutdownImGui() override;
        void newFrameImGui() override;
        void endFrameImGui() override;
        void viewportImGui() override;

    private:
        GLFWwindow* m_glfwWindow = nullptr;

        void initCallbacks();

    };
}

