//
// Created by pierr on 01/04/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/Project.h"

namespace Astra::UI::App {

    class MetricToolPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = "MetricToolPanel";

        MetricToolPanel() : APanel(NAME, UI::Core::PanelType::TOOLS) { m_isOpen = false; }

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override {
            if (m_isOpen && !UI::Core::Project::CurrentProject()->isIsFullScreen()) {
                ImGui::ShowMetricsWindow(&m_isOpen);
            }
        }

    protected:
        void drawPanelContent() override { /* do nothing */ }
    };

    class DebugToolPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = "DebugToolPanel";

        DebugToolPanel() : APanel(NAME, UI::Core::PanelType::TOOLS) { m_isOpen = false; }

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override {
            if (m_isOpen && !UI::Core::Project::CurrentProject()->isIsFullScreen()) {
                ImGui::ShowDebugLogWindow(&m_isOpen);
            }
        }

    protected:
        void drawPanelContent() override { /* do nothing */ }
    };

    class StackToolPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = "StackToolPanel";

        StackToolPanel() : APanel(NAME, UI::Core::PanelType::TOOLS) { m_isOpen = false; }

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override {
            if (m_isOpen && !UI::Core::Project::CurrentProject()->isIsFullScreen()) {
                ImGui::ShowStackToolWindow(&m_isOpen);
            }
        }

    protected:
        void drawPanelContent() override { /* do nothing */ }
    };

    class StyleToolPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = "StyleToolPanel";

        StyleToolPanel() : APanel(NAME, UI::Core::PanelType::TOOLS) { m_isOpen = false; }

        void drawPanelContent() override {
            ImGui::ShowStyleEditor();
        }
    };

}
