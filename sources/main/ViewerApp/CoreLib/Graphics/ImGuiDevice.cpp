//
// Created by pierr on 15/03/2023.
//

#include "ImGuiDevice.h"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

#include "Commons/Profiling.h"

namespace Astra::UI::Core {

    static constexpr const char* glsl_version = "#version 130";

    ImGuiDevice::ImGuiDevice(AWindow* pWindow) : m_window{pWindow} {
        m_window->initImGui();
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    ImGuiDevice::~ImGuiDevice() {
        ImGui_ImplOpenGL3_Shutdown();
        m_window->shutdownImGui();
    }

    void ImGuiDevice::NewFrame() {
        ENGINE_PROFILE_FUNCTION();

        ImGui_ImplOpenGL3_NewFrame();
        m_window->newFrameImGui();
    }

    void ImGuiDevice::EndFrame() const {
        ENGINE_PROFILE_FUNCTION();
        static const auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        const ImGuiIO& io = ImGui::GetIO();

        // si bug bizarre sur glfw, remplacer DisplaySize par glfwGetFrameBuffer
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            m_window->viewportImGui();
        }

        m_window->endFrameImGui();
    }
}
