//
// Created by pierr on 16/03/2023.
//
#include "EntitiesPanel.h"

#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "Commons/Profiling.h"

#include "ViewerApp/CoreLib/CoreEngine.h"

#include "imgui.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/Custom/Serialization/EntitiesLoader.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {

    void EntitiesPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();
        Core::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[Core::Fonts::MONO_BOLD]);

        TipsText::Get().Show("ENTITIES_PANEL_INFO");

        if (!m_currentEngine || !m_currentEngine->IsCoreValid()) {
            ImGui::Text(I18N::Get("NO_ENGINE_SELECTED"));
            return;
        }

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        dragDropTargetEntitiesConfig(isCpuRunning);

        UI::Core::Toggle(I18N::Get("SHOW_SECONDARY"), AstraProject::CurrentProject()->getSettings().DisplaySecondaryEntities);
        ImGui::SameLine();
        drawSubMenu(isCpuRunning);

        static float h = 300;
        static float hs = 200;

        const float tmpV = h + (!AstraProject::CurrentProject()->getSettings().DisplaySecondaryEntities ? hs : 0);

        Core::ScopedStyle btnStyle(ImGuiStyleVar_FramePadding, ImVec2(4,3));

        ImGui::BeginChild("RegisterWindow", ImVec2(0, tmpV), true);
        ImGui::SeparatorText(myFormat(ICON_FA_HASHTAG " {}", I18N::Get("REGISTERS")).c_str());
        drawRegistersTable(isCpuRunning, m_currentEngine->GetEntities().GetRegisters());
        ImGui::EndChild();

        ImGui::InvisibleButton("hsplitter", ImVec2(-1, 6.0f));
        if (ImGui::IsItemActive())
            h += ImGui::GetIO().MouseDelta.y;
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }

        if (AstraProject::CurrentProject()->getSettings().DisplaySecondaryEntities) {
            ImGui::BeginChild("RegisterSecondaryWindow", ImVec2(0, hs), true);
            ImGui::SeparatorText(myFormat(ICON_FA_HASHTAG " {}", I18N::Get("SECONDARY_REGISTERS")).c_str());
            drawRegistersTable(isCpuRunning, m_currentEngine->GetEntities().GetSecondaryRegisters());
            ImGui::EndChild();

            ImGui::InvisibleButton("hsplitter2", ImVec2(-1, 6.0f));
            if (ImGui::IsItemActive())
                hs += ImGui::GetIO().MouseDelta.y;
            if (ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            }
        }

        ImGui::BeginChild("FlagsWindow", ImVec2(0, 0), true);
        ImGui::SeparatorText(myFormat(ICON_FA_FLAG " {}", I18N::Get("FLAGS")).c_str());
        drawFlagsTable(isCpuRunning);
        ImGui::EndChild();
    }

    void EntitiesPanel::drawSubMenu(bool isCpuRunning) const {
        ENGINE_PROFILE_FUNCTION();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 30);
        if (UI::Core::Widgets::IconButton(ICON_FA_GEAR)) {
            ImGui::OpenPopup("EntitiesSettings");
        }
        UI::Core::SetTooltip(I18N::Get("ENTITIES_SETTINGS"));

        if (UI::Core::BeginPopup("EntitiesSettings")) {

            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            if (ImGui::MenuItem(I18N::Get("RESET"))) {
                LOG_DEBUG("EntitiesPanel: clear entities");
                m_currentEngine->Reset();
            }

            if (ImGui::MenuItem(I18N::Get("SAVE"))) {
                saveEntitiesValue();
            }

            if (ImGui::MenuItem(I18N::Get("LOAD"))) {
                loadEntitiesValue();
            }

            if (isCpuRunning) {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }
            UI::Core::EndPopup();
        }
    }

    void EntitiesPanel::drawRegistersTable(bool pIsRunning, const CPU::Core::RegistersInfo& registersList) const {
        ENGINE_PROFILE_FUNCTION();

        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                      ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("RegTable", 3, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(I18N::Get("NAME"));
            ImGui::TableSetupColumn(I18N::Get("VALUE"));
            ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            int i = 0;
            for (const auto& [regName, reg]: registersList) {
                drawRegistersTableItem(i, regName, reg.get(), pIsRunning);
                i++;
            }

            ImGui::EndTable();
        }
    }

    void EntitiesPanel::drawRegistersTableItem(int i, const std::string& regName, CPU::Core::IRegister* reg, bool isRunning) {
        ENGINE_PROFILE_FUNCTION();

        ImGui::TableNextRow();
        ImGui::PushID(i);

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "%s", regName.c_str());

        ImGui::TableNextColumn();
        if (ImGui::Selectable("##cell", false) && !isRunning) {
            ImGui::OpenPopup("RegisterEdit");
        }
        ImGui::SameLine();

        auto currentValue = reg->GetValue();
        const auto fmt = myFormat("0x{}", reg->GetFormat());

        if (currentValue == 0) {
            ImGui::TextDisabled(fmt.c_str(), currentValue);
        } else {
            ImGui::Text(fmt.c_str(), currentValue);
        }
        UI::Core::SetTooltip(myFormat("decimal: {}", currentValue), -1);

        ImGui::TableNextColumn();

        if (isRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (UI::Core::Widgets::IconButton(ICON_FA_CIRCLE_PLUS)) {
            reg->SetValue(currentValue + 1);
        }
        ImGui::SameLine(0, 1);
        if (UI::Core::Widgets::IconButton(ICON_FA_CIRCLE_MINUS)) {
            reg->SetValue(currentValue - 1);
        }
        ImGui::SameLine(0, 1);
        if (UI::Core::Widgets::IconButton(ICON_FA_TRASH_CAN)) {
            reg->SetValue(0);
        }

        if (isRunning) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        if (!isRunning && ImGui::BeginPopup("RegisterEdit")) {
            auto newValue = currentValue;

            ImGui::SetKeyboardFocusHere();
            const int flags = ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
            if (ImGui::InputScalar("##val", ImGuiDataType_U64, &newValue, nullptr, nullptr, reg->GetFormat(), flags) &&
                newValue != currentValue) {
                reg->SetValue(newValue);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    void EntitiesPanel::drawFlagsTable(bool pIsRunning) const {
        ENGINE_PROFILE_FUNCTION();

        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                      ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("FlagsTable", 2, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableSetupColumn(I18N::Get("NAME"));
            ImGui::TableSetupColumn(I18N::Get("VALUE"), ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableHeadersRow();

            int i = 0;
            for (const auto& [flagName, flagValue]: m_currentEngine->GetEntities().GetFlags()) {
                drawFlagsTableItem(i, flagName, *flagValue, pIsRunning);
                i++;
            }

            ImGui::EndTable();
        }
    }

    void EntitiesPanel::drawFlagsTableItem(int i, const std::string& flagName, CPU::Core::IFlagRegister& flagValue, bool isRunning) {
        ENGINE_PROFILE_FUNCTION();

        ImGui::TableNextRow();
        ImGui::PushID(i);

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "%s", flagName.c_str());

        if (isRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        ImGui::TableNextColumn();
        bool val = flagValue.GetValue();
        if (UI::Core::Toggle("##value", val)) {
            flagValue.SetValue(val);
        }

        if (isRunning) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        ImGui::PopID();
    }

    void EntitiesPanel::saveEntitiesValue() const {
        ENGINE_PROFILE_FUNCTION();
        LOG_DEBUG("EntitiesPanel: saveEntitiesValue");

        auto res = UI::Core::FileManager::SaveFile(I18N::Get("SAVE_REGISTERS"), "Registers (*.reg)", {"*.reg"}, "dump.reg");
        if (res.empty()) {
            LOG_WARN("Invalid registers value file. ignored");
            return;
        }

        LOG_INFO("EntitiesPanel: saving value to {}", res);

        EntitiesLoader::saveValues(m_currentEngine, res);

        LOG_DEBUG("EntitiesPanel: saveEntitiesValue end");
    }

    void EntitiesPanel::loadEntitiesValue() const {
        LOG_DEBUG("EntitiesPanel: loadEntitiesValue");
        ENGINE_PROFILE_FUNCTION();

        auto res = UI::Core::FileManager::OpenFile(I18N::Get("LOAD_REGISTERS"), "Entities (*.reg)", {"*.reg"}, false);
        if (res.empty()) {
            LOG_WARN("Invalid entities value file. ignored");
            return;
        }

        LOG_INFO("EntitiesPanel: loading value from {}", res);

        EntitiesLoader::loadConfig(m_currentEngine, res);

        LOG_DEBUG("EntitiesPanel: loadEntitiesValue end");
    }

    void EntitiesPanel::dragDropTargetEntitiesConfig(bool isRunning) const {
        if (!isRunning && ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("config_payload")) {
                const auto& metadata = UI::Core::AssetManager::Get().GetMetadata(*(UUID*) payload->Data);

                if (metadata.isValid()) {
                    EntitiesLoader::loadConfig(m_currentEngine, AstraProject::CurrentProject()->rootDirectory / metadata.FilePath);
                }
            }

            ImGui::EndDragDropTarget();
        }
    }

    void EntitiesPanel::OnEvent(UI::Core::AEvent& pEvent) {
        if (pEvent.GetEventType() == UI::Core::EventType::CpuChanged) {
            m_currentEngine = AstraProject::CurrentProject()->getCurrentEngine();
        }
    }
}
