//
// Created by pierr on 16/08/2023.
//

#include "CustomTitleBar.h"

#include "ViewerApp/Custom/Panels/Views/EditorPanel.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"

#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "ViewerResources.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "Commons/Profiling.h"
#include "ViewerApp/Custom/CustomEvents/ProjectEvents.h"
#include "ViewerApp/Custom/Serialization/EntitiesLoader.h"
#include "ViewerConstants.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "imspinner.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"

namespace Astra::UI::App {

    void CustomTitleBar::drawMenuItems(bool& menuOpen) {
        if (!AstraProject::CurrentProject()) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        drawMenuFile(menuOpen);
        drawTooltipsOnNoProject();
        drawMenuEdit(menuOpen);
        drawTooltipsOnNoProject();
        drawMenuView(menuOpen);
        drawTooltipsOnNoProject();
        drawMenuTools(menuOpen);
        drawTooltipsOnNoProject();
        drawMenuHelp(menuOpen);
        drawTooltipsOnNoProject();

        if (!AstraProject::CurrentProject()) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
    }

    void CustomTitleBar::drawTooltipsOnNoProject() {
        if (!AstraProject::CurrentProject()) {
            Core::SetTooltip(I18N::Get("WAIT_PROJECT_INIT"), -1);
        }
    }

    static bool pushDarkTextIfActive(const char* menuName) {
        if (ImGui::IsPopupOpen(menuName)) {
            ImGui::PushStyleColor(ImGuiCol_Text, UI::Core::Colors::backgroundDark);
            return true;
        }
        return false;
    }

    static void popItemHighlight(bool& menuOpen) {
        if (menuOpen) {
            ImGui::PopStyleColor(3);
            menuOpen = false;
        }
    }

    void CustomTitleBar::drawMenuFile(bool& menuOpen) {
        bool colourPushed = pushDarkTextIfActive(I18N::Get("FILE"));

        bool openNewProjectModal = false;
        bool openCoreLibModal = false;
        bool openSettingsModal = false;

        if (ImGui::BeginMenu(I18N::Get("FILE"))) {
            popItemHighlight(menuOpen);
            colourPushed = false;

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HOVERED_COLUMN);

            if (ImGui::MenuItem(myFormat("   {}...", I18N::Get("CREATE_PROJECT")).c_str())) {
                openNewProjectModal = true;
            }
            if (ImGui::MenuItem(myFormat(ICON_FA_FOLDER_OPEN " {}...", I18N::Get("OPEN_PROJECT")).c_str(), "Ctrl + O")) {
                const auto project = UI::Core::FileManager::OpenFolder(I18N::Get("OPEN_PROJECT"), ".");
                if (!project.empty()) {
                    AstraProject::LoadProject(project, true);
                }
            }

            if (ImGui::BeginMenu(myFormat("   {}...", I18N::Get("RECENT_PROJECT")).c_str())) {
                for (const auto& projectInfo: m_initPanel->GetProjectsHistory()) {
                    if (ImGui::MenuItem(projectInfo->name.c_str())) {
                        AstraProject::LoadProject(projectInfo->path);
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(myFormat(ICON_FA_FLOPPY_DISK " {}", I18N::Get("SAVE_PROJECT")).c_str())) {
                AstraProject::CurrentProject()->SaveProject();
            }

            if (ImGui::MenuItem(myFormat(ICON_FA_CIRCLE_XMARK " {}", I18N::Get("CLOSE_PROJECT")).c_str())) {
                Core::Events::Get().OnEvent<Core::DelayedActionEvent>("TitleBar", [](){
                    const auto projectName = AstraProject::CurrentProject()->getProjectName();
                    AstraProject::SetCurrentProject(nullptr);
                    Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Info, I18N::Get("PROJECT_CLOSED"), projectName));
                });
            }

            ImGui::Separator();

            if (ImGui::MenuItem(myFormat(ICON_FA_HAMMER " {}", I18N::Get("CORE_LIB_RELOAD")).c_str())) {
                Core::AsyncJob::Get().PushTask([]() {
                    CPU::Core::CoreLibManager::Get().ReloadAllLibrary();
                    const auto path = AstraProject::CurrentProject()->rootDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::REG_DUMP_DIR;
                    for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                        EntitiesLoader::loadConfig(engine, path / myFormat("{}.yml", engineUUID));
                    }
                });
            }

            if (ImGui::MenuItem(myFormat(ICON_FA_HAMMER " {}", I18N::Get("CORE_LIB_SHOW")).c_str())) {
                openCoreLibModal = true;
            }

            if (ImGui::MenuItem(myFormat(ICON_FA_SLIDERS " {}", I18N::Get("SETTINGS")).c_str(), "Ctrl + Alt + S")) {
                openSettingsModal = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem(myFormat("   {}", I18N::Get("EXIT")).c_str(), "Alt + F4")) {
                UI::Core::Events::Get().OnEvent<UI::Core::WindowCloseEvent>();
            }

            ImGui::PopStyleColor();

            ImGui::EndMenu();
        }

        if (colourPushed) {
            ImGui::PopStyleColor();
        }

        if (openNewProjectModal) {
            m_newProjectPanel->Open();
        }

        if (openCoreLibModal) {
            m_coreLibModal->Open();
        }

        if (openSettingsModal) {
            m_settingsModal->Open();
        }

        m_newProjectPanel->OnImGuiRender();
        m_coreLibModal->OnImGuiRender();
        m_settingsModal->OnImGuiRender();
    }

    void CustomTitleBar::drawMenuEdit(bool& menuOpen) {
        bool colourPushed = pushDarkTextIfActive(I18N::Get("EDIT"));

        if (ImGui::BeginMenu(I18N::Get("EDIT"))) {
            popItemHighlight(menuOpen);
            colourPushed = false;

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HOVERED_COLUMN);
            bool isFullScreen = UI::Core::Project::CurrentProject()->isIsFullScreen();
            if (ImGui::Checkbox(I18N::Get("FULLSCREEN_MODE"), &isFullScreen)) {
                UI::Core::Project::CurrentProject()->setIsFullScreen(isFullScreen);
            }

            ImGui::MenuItem("NOT IMPLEMENTED", nullptr);

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        if (colourPushed) {
            ImGui::PopStyleColor();
        }
    }

    void CustomTitleBar::drawMenuView(bool& menuOpen) const {
        bool colourPushed = pushDarkTextIfActive(I18N::Get("VIEW"));

        if (ImGui::BeginMenu(I18N::Get("VIEW"))) {
            popItemHighlight(menuOpen);
            colourPushed = false;

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HOVERED_COLUMN);
            for (const auto& [name, panel]: m_panelsStack) {
                if (panel->Type == UI::Core::PanelType::VIEW && ImGui::MenuItem(name.c_str())) {
                    panel->Open();
                }
            }

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        if (colourPushed) {
            ImGui::PopStyleColor();
        }
    }

    void CustomTitleBar::drawMenuTools(bool& menuOpen) const {
        bool colourPushed = pushDarkTextIfActive(I18N::Get("TOOLS"));

        if (ImGui::BeginMenu(I18N::Get("TOOLS"))) {
            popItemHighlight(menuOpen);
            colourPushed = false;

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HOVERED_COLUMN);

            for (const auto& [name, panel]: m_panelsStack) {
                if (panel->Type == UI::Core::PanelType::TOOLS && ImGui::MenuItem(name.c_str())) {
                    panel->Open();
                }
            }

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        if (colourPushed) {
            ImGui::PopStyleColor();
        }
    }

    void CustomTitleBar::drawMenuHelp(bool& menuOpen) {
        bool colourPushed = pushDarkTextIfActive(I18N::Get("HELP"));
        bool openAbout = false;

        if (ImGui::BeginMenu(I18N::Get("HELP"))) {
            popItemHighlight(menuOpen);
            colourPushed = false;

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HOVERED_COLUMN);

            if (ImGui::MenuItem(myFormat(ICON_FA_ADDRESS_CARD " {}", I18N::Get("ABOUT")).c_str())) {
                openAbout = true;
            }

            ImGui::PopStyleColor();
            ImGui::EndMenu();
        }

        if (colourPushed) {
            ImGui::PopStyleColor();
        }

        if (openAbout) {
            m_aboutModal.OpenMessage({
                                             "Developed by Pierre REBUT",
                                             I18N::Get("LICENCE"),
                                             myFormat("Version : {}", ASTRA_VERSION)
                                     });
        }

        m_aboutModal.OnImGuiRender();
    }

    void CustomTitleBar::drawCustomStatus() {
        ENGINE_PROFILE_FUNCTION();

        if (!CPU::Core::EngineManager::Get().isInit()) {
            return;
        }

        const auto& engineManager = CPU::Core::EngineManager::Get();
        auto& runManager = CPU::Core::RunManager::Get();

        {
            if (ImGui::InvisibleButton("SelectCPU", ImVec2(76, 32))) {
                ImGui::OpenPopup("SelectCPUPopup");
            }
            const auto& currentEngine = AstraProject::CurrentProject()->getCurrentEngine();
            UI::Core::DrawButtonTextImage(currentEngine ? currentEngine->GetName().c_str() : "#UNDEF", ViewerResources::SelectCpuIcon,
                                          IM_COL32(255, 255, 255, 255),
                                          IM_COL32(102, 204, 163, 200),
                                          IM_COL32(102, 204, 163, 120),
                                          UI::Core::RectExpanded(UI::Core::GetItemRect(), 1.0f, 1.0f));
            UI::Core::SetTooltip(I18N::Get("SELECT_CPU"), 0.5);

            if (ImGui::BeginPopup("SelectCPUPopup")) {
                for (const auto& [engineUUID, engine]: engineManager.getEngines()) {
                    if (ImGui::Selectable(engine->GetName().c_str(), engine == currentEngine)) {
                        AstraProject::CurrentProject()->setCurrentEngine(engine);
                        UI::Core::Events::Get().OnEvent<CpuChangedEvent>();
                    }
                }
                ImGui::EndPopup();
            }

        }

        const bool isRunning = runManager.isRunning();
        auto iconColor = IM_COL32(255, 255, 255, 255);

        if (isRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            iconColor = IM_COL32(100, 100, 100, 255);
        }

        {
            ImGui::SameLine(0, 3);

            if (ImGui::InvisibleButton("Run", ImVec2(32, 32))) {
                if (runManager.Run(AstraProject::CurrentProject()->getCurrentEngine())) {
                    UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::CpuRun);
                    if (AstraProject::CurrentProject()->getSettings().LogAutoCleanOnRun) {
                        AstraProject::CurrentProject()->GetData().logMessages.clear();
                    }
                }
            }
            UI::Core::DrawButtonImage(ViewerResources::RunIcon, iconColor,
                                      IM_COL32(102, 204, 163, 200),
                                      IM_COL32(102, 204, 163, 120),
                                      UI::Core::RectExpanded(UI::Core::GetItemRect(), 1.0f, 1.0f));
            UI::Core::SetTooltip(I18N::Get("RUN_CPUS"), 0.5);
        }

        {
            ImGui::SameLine(0, 3);

            if (ImGui::InvisibleButton("Step", ImVec2(32, 32)) && AstraProject::CurrentProject()->getCurrentEngine()) {
                UI::Core::AsyncJob::Get().PushTask([](){
                    CPU::Core::RunManager::Get().ExecuteStep(AstraProject::CurrentProject()->getCurrentEngine(), 1);
                });
            }
            UI::Core::DrawButtonImage(ViewerResources::StepIcon, iconColor,
                                      IM_COL32(102, 204, 163, 200),
                                      IM_COL32(102, 204, 163, 120),
                                      UI::Core::RectExpanded(UI::Core::GetItemRect(), 1.0f, 1.0f));
            UI::Core::SetTooltip(I18N::Get("STEP_CPUS"), 0.5);
        }

        if (isRunning) {
            ImGui::PopItemFlag();
            iconColor = IM_COL32(255, 255, 255, 255);
        } else {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            iconColor = IM_COL32(100, 100, 100, 255);
        }

        {
            ImGui::SameLine(0, 3);

            if (ImGui::InvisibleButton("Stop", ImVec2(32, 32))) {
                runManager.Stop();
                UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::CpuStop);
            }
            UI::Core::DrawButtonImage(ViewerResources::StopIcon, iconColor,
                                      IM_COL32(102, 204, 163, 200),
                                      IM_COL32(102, 204, 163, 120),
                                      UI::Core::RectExpanded(UI::Core::GetItemRect(), 1.0f, 1.0f));
            UI::Core::SetTooltip(I18N::Get("STOP_CPUS"), 0.5);
        }

        if (!isRunning) {
            ImGui::PopItemFlag();
        }
    }

    void CustomTitleBar::drawProjectNameInfo() {
        {
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical | ImGuiSeparatorFlags_SpanAllColumns);

            const auto [statusName, statusColor, spinner] = getStatusValue();

            ImGui::SameLine();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
            ImSpinner::SpinnerClock("##running", 8, 1, ImColor(255, 0, 0), ImSpinner::white, 2 * spinner);
            ImGui::PopStyleVar();

            UI::Core::ScopedColour textColour(ImGuiCol_Text, statusColor);
            ImGui::SameLine();
            ImGui::Text(statusName);
        }
    }

    std::tuple<const char*, int, bool> CustomTitleBar::getStatusValue() {
        if (!CPU::Core::EngineManager::Get().isInit()) {
            return {"UNDEFINED", IM_COL32(140, 6, 158, 255), false};
        }

        const auto& runManager = CPU::Core::RunManager::Get();

        if (runManager.isRunning()) {
            return {I18N::Get("RUNNING"), IM_COL32(50, 205, 50, 255), true};
        }

        return {I18N::Get("STOPPED"), IM_COL32(244, 68, 78, 255), false};
    }
}