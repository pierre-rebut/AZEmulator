//
// Created by pierr on 16/03/2023.
//
#include "MemoryPanel.h"

#include <fstream>

#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "CpuEngine/manager/running/RunManager.h"

#include "Commons/Profiling.h"
#include "imgui.h"

#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "ViewerApp/CoreLib/ImGuiUtils.h"
#include "Commons/utils/Utils.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "ViewerApp/Custom/Serialization/MemoryLoader.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {

    void MemoryPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        TipsText::Get().Show("MEMORY_PANEL_INFO");

        static const ImGuiTableFlags flags = ImGuiTabBarFlags_Reorderable |
                                             ImGuiTabBarFlags_FittingPolicyScroll;

        if (ImGui::BeginTabBar("##tabs", flags)) {
            if (ImGui::TabItemButton(ICON_FA_PLUS, ImGuiTabItemFlags_Trailing)) {
                m_inputsModal.OpenInput(I18N::Get("NEW_BUS"), {{I18N::Get("NAME"), "main"},
                                                      {I18N::Get("SIZE"), "0x10000"}}, [](const auto& result) {
                    auto& databusManager = CPU::Core::DataBusManager::Get();
                    for (const auto& [databusUUID, databus] : databusManager.GetDataBuses()) {
                        if (databus->GetName() == result.at(0).second) {
                            return false;
                        }
                    }

                    databusManager.CreateNewBus(result.at(0).second, Utils::ParseNumber(result.at(1).second));
                    return true;
                });
            }

            m_inputsModal.OnImGuiRender();

            drawMemTab();

            ImGui::EndTabBar();
        }
    }

    void MemoryPanel::drawMemTab() {
        ENGINE_PROFILE_FUNCTION();
        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        for (const auto& [busUUID, dataBus]: CPU::Core::DataBusManager::Get().GetDataBuses()) {
            if (ImGui::BeginTabItem(dataBus->GetName().c_str(), nullptr, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton)) {
                drawMenuBar(dataBus, isCpuRunning);

                drawHexTable(dataBus, isCpuRunning);

                dragDropTarget(dataBus, isCpuRunning);

                ImGui::EndTabItem();
            }
        }
    }

    void MemoryPanel::drawMenuBar(const Ref<CPU::Core::DataBus>& dataBus, bool isCpuRunning) {
        ENGINE_PROFILE_FUNCTION();

        ImGui::SetNextItemWidth(120);
        this->m_searchUpdated = UI::Core::Widgets::SearchWidget<0, unsigned int>("searchBar", this->m_searchBuffer);

        const auto btnStr = myFormat(ICON_FA_LINK " {}", I18N::Get("DEVICES"));
        ImGui::SameLine(ImGui::GetWindowWidth() - (ImGui::CalcTextSize(btnStr.c_str()).x + 80));

        if (ImGui::Button(btnStr.c_str())) {
            m_configModal->OpenEdit(dataBus);
        }

        ImGui::SameLine();

        if (UI::Core::Widgets::IconButton(ICON_FA_GEAR)) {
            ImGui::OpenPopup("MemorySettings");
        }
        UI::Core::SetTooltip(I18N::Get("MEMORY_SETTINGS"));

        m_configModal->OnImGuiRender();
        bool openLoadMenu = false;
        bool openRenameMenu = false;
        bool openDeleteMenu = false;

        if (UI::Core::BeginPopup("MemorySettings")) {
            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            if (ImGui::MenuItem(I18N::Get("MEMORY_CLEAR"))) {
                LOG_INFO("Clear memory");
                dataBus->Reset();
            }
            if (ImGui::MenuItem(I18N::Get("MEMORY_DUMP"))) {
                auto filepath = UI::Core::FileManager::SaveFile(I18N::Get("MEMORY_DUMP"), "Dump file (*.mem)", {"*.mem"}, "");
                if (!filepath.empty()) {
                    MemoryLoader::saveMemory(dataBus, filepath);
                }
            }
            if (ImGui::MenuItem(I18N::Get("MEMORY_LOAD"))) {
                openLoadMenu = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem(myFormat(ICON_FA_PEN " {}", I18N::Get("RENAME")).c_str())) {
                openRenameMenu = true;
            }

            bool isReadOnly = dataBus->IsReadOnly();
            if (UI::Core::Toggle(I18N::Get("READ_ONLY"), isReadOnly)) {
                dataBus->SetReadOnly(isReadOnly);
            }

            if (ImGui::MenuItem(myFormat(ICON_FA_TRASH " {}", I18N::Get("REMOVE")).c_str())) {
                openDeleteMenu = true;
            }

            if (isCpuRunning) {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }

            UI::Core::EndPopup();
        }

        if (openRenameMenu) {
            m_inputModal.OpenInput(I18N::Get("MEMORY_RENAME"), dataBus->GetName(), [&dataBus](const std::string& newName){
                dataBus->SetName(newName);
            });
        }
        m_inputModal.OnImGuiRender();

        if (openDeleteMenu) {
            m_questionModal.OpenQuestion(myFormat(I18N::Get("REMOVE_QUESTION"), dataBus->GetName()), [&dataBus](bool res){
                if (res) {
                    UI::Core::Events::Get().OnEvent<UI::Core::DelayedActionEvent>("MemoryPanel", [&dataBus]() {
                        CPU::Core::DataBusManager::Get().RemoveDataBus(dataBus);
                    });
                }
            });
        }
        m_questionModal.OnImGuiRender();

        if (openLoadMenu) {
            auto filepath = UI::Core::FileManager::OpenFile(I18N::Get("OPEN_FILE"), "Dump file (*.mem)", {"*.mem"});
            if (!filepath.empty()) {
                LOG_INFO("Open load memory from file {}", filepath);
                tmpDropFile = filepath;
                openQuestionLoadMemory(dataBus);
            }
        }
    }

    void MemoryPanel::openQuestionLoadMemory(const Ref<CPU::Core::DataBus>& dataBus) {
        m_inputsModal.OpenInput(I18N::Get("LOAD_MEMORY"), {{I18N::Get("BEGIN_ADDRESS"), "0"},
                                              {I18N::Get("OFFSET_IN_FILE"), "0"}}, [this, &dataBus](const auto& result) {
            const auto& pos = result.at(0).second;
            const auto& offset = result.at(1).second;

            LOG_INFO("Load memory from file {} starting at addr {} with offset {} in file", tmpDropFile, pos, offset);
            return MemoryLoader::loadMemory(dataBus, tmpDropFile, Utils::ParseNumber(pos), Utils::ParseNumber(offset));
        });
    }

    void MemoryPanel::dragDropTarget(const Ref<CPU::Core::DataBus>& dataBus, bool isCpuRunning) {
        if (isCpuRunning) {
            return;
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload")) {
                const auto& metadata = UI::Core::AssetManager::Get().GetMetadata(*(UUID*) payload->Data);

                if (metadata.isValid()) {
                    tmpDropFile = AstraProject::CurrentProject()->rootDirectory / metadata.FilePath;

                    openQuestionLoadMemory(dataBus);
                }
            }

            ImGui::EndDragDropTarget();
        }

        m_inputsModal.OnImGuiRender();
    }

    void MemoryPanel::drawHexTable(const Ref<CPU::Core::DataBus>& dataBus, bool isCpuRunning) {
        ENGINE_PROFILE_FUNCTION();
        UI::Core::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::MONO_BOLD]);

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                       ImGuiTableFlags_BordersV;

        if (ImGui::BeginTable("MemTable", 18, flags)) {

            ImGui::TableSetupScrollFreeze(1, 1); // Make top row always visible
            ImGui::TableSetupColumn(I18N::Get("ADDRESS"), ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn(" 0", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 1", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 2", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 3", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 4", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 5", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 6", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 7", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 8", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" 9", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" A", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" B", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" C", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" D", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" E", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn(" F", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn("ASCII");
            ImGui::TableHeadersRow();

            int currentPCAddr = -2;
            if (m_currentEngine) {
                const auto& programCounter =  m_currentEngine->GetEntities().GetProgramCounterInfo();

                if (programCounter.isValid && programCounter.dataBus->GetValue() == dataBus) {
                    currentPCAddr = programCounter.pcFn();
                }
            }

            ImGuiListClipper clipper;
            clipper.Begin((int) dataBus->GetBusSize() / 16, ImGui::GetTextLineHeightWithSpacing() * 1.50f);
            while (clipper.Step()) {

                auto it = std::ranges::find_if(dataBus->GetLinkedDevices().begin(), dataBus->GetLinkedDevices().end(), [&clipper](const auto& device) {
                    return device.addressHigh >= (clipper.DisplayStart * 16);
                });

                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                    ImGui::TableNextRow();
                    UI::Core::ImGuiUtils::forceColumnMaxSize(18, row, clipper.DisplayStart);

                    ImGui::TableSetColumnIndex(0);

                    auto addr = row * 16;
                    ImGui::AlignTextToFramePadding();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "%04X-%04X:", addr >> 16, addr & 0xFFFF);

                    std::ostringstream asciiValue;

                    drawHexTableItem(dataBus, isCpuRunning, currentPCAddr, addr, it, asciiValue);

                    ImGui::TableSetColumnIndex(17);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "%s", asciiValue.str().c_str());
                }
            }

            if (m_searchUpdated) {
                ImGui::SetScrollY((m_searchBuffer & 0xFFFFF0) * 2.25f);
            }

            ImGui::EndTable();
        }
    }

    void MemoryPanel::drawHexTableItem(const Ref<CPU::Core::DataBus>& dataBus, bool pIsRunning, int currentPCAddr, int addr,
                                       std::list<CPU::Core::ConnectedDevice>::const_iterator& it, std::ostringstream& asciiValue) {
        ENGINE_PROFILE_FUNCTION();

        for (int column = 0; column < 16; column++) {
            ImGui::TableSetColumnIndex(column + 1);
            auto currentAddr = addr + column;
            ImGui::PushID(currentAddr);

            if (it != dataBus->GetLinkedDevices().end() && it->addressHigh < currentAddr) {
                it++;
            }

            int currentVal = 0;
            ImU32 cellColor = 0;
            std::string tooltipContent;

            if (currentAddr == currentPCAddr) {
                cellColor = IM_COL32(10, 128, 10, 200);
            }

            if (it != dataBus->GetLinkedDevices().end() && it->isAddressValid(currentAddr)) {

                if (cellColor == 0) {
                    const auto u = it->device->deviceUUID;
                    cellColor = IM_COL32(u % 256, (u >> 8) % 256, (u >> 16) % 256, 40);
                }

                currentVal = it->device->FetchReadOnly(currentAddr - it->addressLow + it->internalAddress);
                tooltipContent = myFormat("{} (UUID: {})", it->device->GetName(), it->device->deviceUUID);
            } else {
                currentVal = dataBus->GetRamMemory().FetchReadOnly(currentAddr);
            }

            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellColor);

            if (currentVal == -1) {
                ImGui::Text("XX");
            } else {
                currentVal = drawRamCell(dataBus, pIsRunning, currentAddr, currentVal);
            }

            if (!tooltipContent.empty()) {
                UI::Core::SetTooltip(tooltipContent, -1);
            }

            asciiValue.put((char) (currentVal > 0x20 && currentVal < 0x7F ? currentVal : '.'));
            ImGui::PopID();
        }
    }

    BYTE MemoryPanel::drawRamCell(const Ref<CPU::Core::DataBus>& dataBus, bool pIsRunning, int currentAddr, BYTE currentVal) {
        if (ImGui::Selectable("##cell", false) && !pIsRunning) {
            ImGui::OpenPopup("MemoryEdit");
        }

        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
        if (currentVal == 0) {
            ImGui::TextDisabled("%02X", currentVal);
        } else {
            ImGui::Text("%02X", currentVal);
        }

        if (ImGui::BeginPopup("MemoryEdit")) {
            BYTE newVal = currentVal;

            ImGui::SetKeyboardFocusHere();
            const ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal |
                                              ImGuiInputTextFlags_EnterReturnsTrue |
                                              ImGuiInputTextFlags_AutoSelectAll;

            if (ImGui::InputScalar("##val", ImGuiDataType_U8, &newVal, nullptr, nullptr, "%02X", flags) && newVal != currentVal) {
                dataBus->Push(CPU::DataFormat::Byte, currentAddr, newVal);
                currentVal = newVal;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        return currentVal;
    }

    void MemoryPanel::OnEvent(UI::Core::AEvent& pEvent) {
        if (pEvent.GetEventType() == UI::Core::EventType::CpuChanged) {
            m_currentEngine = AstraProject::CurrentProject()->getCurrentEngine();
        }
    }
}
