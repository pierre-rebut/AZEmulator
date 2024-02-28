//
// Created by pierr on 28/08/2023.
//

#include "CoreLibModal.h"

#include "imgui.h"

#include "Commons/Profiling.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/Utils.h"

#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "ViewerApp/CoreLib/Resources/Resources.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "ViewerApp/Custom/ViewerConstants.h"
#include "ViewerApp/CoreLib/Resources/CustomTreeNode.h"
#include "ViewerApp/CoreLib/AsyncJob.h"
#include "ViewerApp/Custom/Serialization/EntitiesLoader.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {
    void CoreLibModal::drawPopupContent() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::BeginChild("CoreLibModal", ImVec2(500, 500), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

        drawSubMenu();

        const auto& coreLibs = CPU::Core::CoreLibManager::Get().GetCoreLibs();
        if (coreLibs.empty()) {
            ImGui::Text(I18N::Get("NOTHING_TO_SHOW"));
        } else {
            drawCoreLibs(coreLibs);
        }

        ImGui::EndChild();

        UI::Core::PopupCloseButton();
    }

    void CoreLibModal::drawCoreLibs(const std::map<std::string, Scope<CPU::Core::CoreLibDir>>& coreLibs) {
        int i = 0;

        ImGui::PushStyleColor(ImGuiCol_Header, Core::Colors::groupHeader);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Core::Colors::groupHeader);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, Core::Colors::groupHeader);

        for (const auto& [dirName, dir]: coreLibs) {

            if (ImGui::CollapsingHeader(dirName.c_str())) {

                const static ImGuiTableFlags flags =
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;
                if (ImGui::BeginTable("CoreLibTable", 3, flags)) {
                    ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                    ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 15);
                    ImGui::TableSetupColumn(I18N::Get("NAME"));
                    ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 40);
                    ImGui::TableHeadersRow();

                    for (const auto& [libName, lib]: dir->libs) {
                        drawCoreLibsItem(i, lib);
                        i++;
                    }

                    ImGui::EndTable();
                }
            }
        }

        ImGui::PopStyleColor(3);
    }

    void CoreLibModal::drawCoreLibsItem(int i, const Ref<CPU::Core::CoreLib>& lib) {
        ENGINE_PROFILE_FUNCTION();

        ImGui::TableNextRow();
        ImGui::PushID(i);

        auto colorName = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);

        ImGui::TableNextColumn();
        if (lib->IsLoaded()) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7);
            ImGui::Text(ICON_FA_BOOK_OPEN);
            colorName = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        }

        ImGui::TableNextColumn();

        ImGui::AlignTextToFramePadding();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);

        ImGui::TextColored(colorName, lib->name.c_str());
        UI::Core::SetTooltip(myFormat("{}: {}", I18N::Get("PATH"), lib->path), -1);

        ImGui::TableNextColumn();
        bool isCpuNotRunning = !CPU::Core::RunManager::Get().isRunning();
        if (UI::Core::Widgets::IconButton(ICON_FA_ROTATE) && isCpuNotRunning) {
            Core::AsyncJob::Get().PushTask([lib]() {
                auto res = lib->ReloadLibrary();
                if (!res) {
                    return;
                }

                const auto path = AstraProject::CurrentProject()->rootDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::REG_DUMP_DIR;
                for (const auto& [engineUUID, engine]: lib->GetEngines()) {
                    EntitiesLoader::loadConfig(engine, path / myFormat("{}.yml", engineUUID));
                }
            });
        }

        ImGui::PopID();
    }

    void CoreLibModal::drawSubMenu() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 30);
        if (UI::Core::Widgets::IconButton(ICON_FA_GEAR)) {
            ImGui::OpenPopup("EntitiesSettings");
        }
        UI::Core::SetTooltip(I18N::Get("ENTITIES_SETTINGS"));

        if (UI::Core::BeginPopup("EntitiesSettings")) {

            bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            if (ImGui::MenuItem(I18N::Get("REFRESH_CORE_LIB"))) {
                Core::AsyncJob::Get().PushTask([]() {
                    CPU::Core::CoreLibManager::Get().FindCoreLib(ViewerConstants::CORE_LIB_DIR);
                    const auto path = AstraProject::CurrentProject()->rootDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::REG_DUMP_DIR;
                    for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                        auto res = CPU::Core::CoreLibManager::Get().LoadEngineCoreLib(engine);
                        if (res) {
                            EntitiesLoader::loadConfig(engine, path / myFormat("{}.yml", engineUUID));
                        }
                    }
                });
            }

            if (ImGui::MenuItem(I18N::Get("RELOAD_ALL_LIB"))) {
                Core::AsyncJob::Get().PushTask([]() {
                    CPU::Core::CoreLibManager::Get().ReloadAllLibrary();
                    const auto path = AstraProject::CurrentProject()->rootDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::REG_DUMP_DIR;
                    for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                        EntitiesLoader::loadConfig(engine, path / myFormat("{}.yml", engineUUID));
                    }
                });
            }

            if (isCpuRunning) {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }
            UI::Core::EndPopup();
        }
    }
} // Astra