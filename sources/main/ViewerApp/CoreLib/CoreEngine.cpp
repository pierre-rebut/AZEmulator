//
// Created by pierr on 15/03/2023.
//
#include "CoreEngine.h"

#include "ViewerApp/CoreLib/Plateform/GLFW/GlfwWindow.h"

#include "Timer.h"
#include "Commons/Profiling.h"
#include "Colors.h"
#include "imgui.h"

#include "IconsFontAwesome6.h"
#include "imnodes.h"
#include "imgui_plot/implot.h"
#include "IconsFontAwesome6Brands.h"

namespace Astra::UI::Core {
    CoreEngine::CoreEngine() {
        LOG_INFO("ImGuiEngine: Init");

        CoreEngine::SetSingleton(this);

        m_i18n = CreateScope<I18NImpl>();
        I18NImpl::SetSingleton(m_i18n.get());

        m_window = CreateScope<Glfw::GlfwWindow>(PROJECT_NAME);
        m_window->initWindow();

        initImGui();

        m_device = CreateScope<ImGuiDevice>(m_window.get());

        m_assetManager = CreateScope<AssetManager>();
        AssetManager::SetSingleton(m_assetManager.get());

        m_asyncJob = CreateScope<AsyncJob>();
        AsyncJob::SetSingleton(m_asyncJob.get());

        m_tipsText = CreateScope<TipsText>();
        TipsText::SetSingleton(m_tipsText.get());

        LOG_INFO("ImGuiEngine: Init end");
    }

    CoreEngine::~CoreEngine() {
        LOG_INFO("ImGuiEngine: destroy");

        m_asyncJob = nullptr;
        m_mainScreen = nullptr;
        m_assetManager = nullptr;
        m_tipsText = nullptr;

        m_device = nullptr;
        ImPlot::DestroyContext();
        ImNodes::DestroyContext();
        ImGui::DestroyContext();
        m_window = nullptr;

        m_i18n = nullptr;

        TipsText::SetSingleton(nullptr);
        AsyncJob::SetSingleton(nullptr);
        AssetManager::SetSingleton(nullptr);
        I18NImpl::SetSingleton(nullptr);
        CoreEngine::SetSingleton(nullptr);

        LOG_INFO("ImGuiEngine: destroy end");
    }

    static ImFont* addFont(const std::string_view& fontPath, float fontSize) {
        ImGuiIO& io = ImGui::GetIO();
        auto newFont = io.Fonts->AddFontFromFileTTF(fontPath.data(), fontSize);

        float iconFontSize = fontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = iconFontSize;

        io.Fonts->AddFontFromFileTTF("resources/fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);
        io.Fonts->AddFontFromFileTTF("resources/fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAB, iconFontSize, &icons_config, icons_ranges);

        return newFont;
    }

    void CoreEngine::initImGui() {
        LOG_DEBUG("ImGuiEngine: initImGui");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImNodes::CreateContext();
        ImPlot::CreateContext();

        LOG_TRACE("ImGuiEngine: context created");

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        // FONTS::DEFAULT
        io.FontDefault = addFont("resources/fonts/JetBrainsMono-Regular.ttf", 22);
        // FONTS::SMALL
        addFont("resources/fonts/JetBrainsMono-Regular.ttf", 18);
        // FONTS::BOLD
        addFont("resources/fonts/Roboto/Roboto-Bold.ttf", 18.0f);
        // FONTS::MONO_BOLD
        addFont("resources/fonts/SpaceMono-Bold.ttf", 20.f);
        // FONTS::LARGE
        addFont("resources/fonts/Roboto/Roboto-Regular.ttf", 48.0f);

        LOG_TRACE("ImGuiEngine: fonts created");

        ImGui::StyleColorsDark();
        setDarkThemeColors();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        /// Style
        style.FrameRounding = 2.5f;
        style.FrameBorderSize = 1.0f;
        style.IndentSpacing = 11.0f;
        style.FramePadding = ImVec2(18, 6);


        LOG_DEBUG("ImGuiEngine: initImGui end");
    }

    void CoreEngine::setDarkThemeColors() {
        LOG_DEBUG("ImGuiEngine: setDarkThemeColors");

        auto& style = ImGui::GetStyle();
        auto& colors = style.Colors;

        //========================================================
        /// Colours

        // Headers
        colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::groupHeader);
        colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::groupHeader);
        colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::groupHeader);

        // Buttons
        colors[ImGuiCol_Button] = ImColor(56, 56, 56, 200);
        colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
        colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::propertyField);
        colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::propertyField);
        colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::propertyField);

        // Tabs
        colors[ImGuiCol_Tab] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::titlebar);
        colors[ImGuiCol_TabHovered] = ImColor(255, 225, 135, 30);
        colors[ImGuiCol_TabActive] = ImColor(255, 225, 135, 60);
        colors[ImGuiCol_TabUnfocused] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::titlebar);
        colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

        // Title
        colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::titlebar);
        colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::titlebar);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Resize Grip
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

        // Check Mark
        colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

        // Slider
        colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

        // Text
        colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::text);

        // Checkbox
        colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::text);

        // Separator
        colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::backgroundDark);
        colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::highlight);
        colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

        // Window Background
        colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);
        colors[ImGuiCol_ChildBg] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::background);
        colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::backgroundPopup);
        colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::backgroundDark);

        // Tables
        colors[ImGuiCol_TableHeaderBg] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::groupHeader);
        colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::backgroundDark);

        // Menubar
        colors[ImGuiCol_MenuBarBg] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

        LOG_DEBUG("ImGuiEngine: setDarkThemeColors end");
    }

    void CoreEngine::Run() {
        LOG_INFO("ImGuiEngine: run");

        AstraException::assertV(m_mainScreen.operator bool(), "Main screen not Init");

        Timer timer{};
        ImGui::GetIO().IniFilename = nullptr;

        while (m_mainScreen->isRunning()) {
            ENGINE_PROFILE_FRAME("Astra");

            m_window->processEvents();

            auto frameTime = timer.get();
            timer.reset();

            FrameInfo frameInfo{
                    frameTime,
                    m_frameCount
            };

            //OnEvent
            {
                m_mainScreen->ProcessEvents();
            }

            // OnUpdate
            {
                ENGINE_PROFILE_SCOPE("Layer OnUpdate");
                m_mainScreen->OnUpdate(frameInfo);
            }

            // OnImGuiRender
            {
                ENGINE_PROFILE_SCOPE("Layer OnImGuiRender");

                m_device->NewFrame();
                ImGui::NewFrame();

                m_mainScreen->OnImGuiRender(frameInfo);

                ImGui::Render();
                m_device->EndFrame();
            }

            m_frameCount++;
        }

        LOG_INFO("ImGuiEngine: run end");
    }
}
