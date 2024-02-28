//
// Created by pierr on 19/07/2023.
//

#include <ranges>

#include "ConfigPanel.h"
#include "imgui.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/Custom/CustomEvents/ProjectEvents.h"
#include "ViewerApp/CoreLib/ImGuiUtils.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Resources/Sound.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "CpuEngine/engine/hardwareDevices/impl/DiskDevice.h"

namespace Astra::UI::App {

    void ConfigPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();
        Core::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[Core::Fonts::MONO_BOLD]);

        TipsText::Get().Show("CONFIG_PANEL_INFO");

        ImGui::SetNextItemWidth(150);
        if (ImGui::InputInt(ICON_FA_GAUGE_HIGH, &AstraProject::CurrentProject()->getSettings().CpuSpeed, 10, 100)) {
            if (AstraProject::CurrentProject()->getSettings().CpuSpeed < 1) {
                AstraProject::CurrentProject()->getSettings().CpuSpeed = 1;
            }
            CPU::Core::RunManager::Get().SetSystemClockSpeed(AstraProject::CurrentProject()->getSettings().CpuSpeed);
        }

        Core::SetTooltip(I18N::Get("SPEED_INFO"), -1);

        ImGui::SameLine();

        auto resetStr = myFormat(ICON_FA_POWER_OFF " {}", I18N::Get("RESET"));
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(resetStr.c_str()).x + 36));

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        static bool isResetting = false;
        if (ImGui::Button(resetStr.c_str()) && !isResetting) {
            isResetting = true;
            UI::Core::AsyncJob::Get().PushTask([]() {
                Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Info, I18N::Get("RESET_PENDING")));
                CPU::Core::EngineManager::Get().Reset();
                Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Success, I18N::Get("RESET_SUCCESS")));
                isResetting = false;
            });
        }

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        Core::SetTooltip(I18N::Get("RESET_INFO"), -1);

        static float h = (ImGui::GetContentRegionAvail().y / 2) - 10;

        ImGui::BeginChild("EngineTable", ImVec2(0, h), true);
        ImGui::SeparatorText(myFormat(ICON_FA_MICROCHIP " {}", I18N::Get("ENGINES")).c_str());
        addEngine();
        drawEngineTable();
        ImGui::EndChild();

        ImGui::InvisibleButton("hsplitter", ImVec2(-1, 6.0f));
        if (ImGui::IsItemActive())
            h += ImGui::GetIO().MouseDelta.y;
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }

        ImGui::BeginChild("DeviceChild", ImVec2(0, 0), true);
        ImGui::SeparatorText(myFormat(ICON_FA_PLUG " {}", I18N::Get("DEVICES")).c_str());
        drawDevicesTable();
        ImGui::EndChild();
    }

    void ConfigPanel::addEngine() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::SameLine();

        auto addStr = myFormat(ICON_FA_CIRCLE_PLUS "  {}", I18N::Get("ADD"));
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(addStr.c_str()).x + 60));

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (ImGui::Button(addStr.c_str())) {
            m_inputModal.OpenInput(I18N::Get("ENGINE_NAME"), "cpu", [](const char* engineName){
                CPU::Core::EngineManager::Get().AddEngine(CPU::Core::CpuCreateData{UUIDGen::New(), engineName});
            });
        }

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        m_inputModal.OnImGuiRender();

        ImGui::SameLine();
        ImGui::Text(ICON_FA_CIRCLE_QUESTION);
        UI::Core::SetTooltip(I18N::Get("ENGINE_INFO"), -1);
    }

    void ConfigPanel::drawEngineTable() const {

        static const auto flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                  ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("CpuTable", 8, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 15);
            ImGui::TableSetupColumn(I18N::Get("NAME"), ImGuiTableColumnFlags_WidthFixed, 140);
            ImGui::TableSetupColumn(ICON_FA_HASHTAG);
            ImGui::TableSetupColumn(I18N::Get("STATUS"));
            ImGui::TableSetupColumn(I18N::Get("LIB"), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(I18N::Get("SPEED"));
            ImGui::TableSetupColumn(I18N::Get("FREQUENCY"));
            ImGui::TableSetupColumn(" ");
            ImGui::TableHeadersRow();

            int i = 0;
            for (const auto& [uuid, engine]: Singleton<::Astra::CPU::Core::EngineManager>::Get().getEngines()) {
                drawEngineTableItem(i, engine);
                i++;
            }

            ImGui::EndTable();
        }
    }

    void ConfigPanel::drawEngineTableItem(int i, const Ref<CPU::Core::CpuEngine>& engine) const {
        ENGINE_PROFILE_FUNCTION();

        ImGui::TableNextRow();
        UI::Core::ImGuiUtils::forceColumnMaxSize(8, i, 0);
        ImGui::TableSetColumnIndex(0);

        ImGui::PushID(i);

        if (AstraProject::CurrentProject()->getCurrentEngine() == engine) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7);
            ImGui::Text(ICON_FA_ARROW_RIGHT);
        }

        ImGui::TableNextColumn();

        ImGui::AlignTextToFramePadding();

        ImVec4 colorStatus = ImGui::ColorConvertU32ToFloat4(UI::Core::Colors::text);
        std::string iconStatus = "  ";
        std::string status = "READY";
        std::string statusInfo = "READY_INFO";

        if (!engine->IsCoreInit()) {
            colorStatus = ImVec4(0.86f, 0.56f, 0.07f, 1.0f);
            status = "INIT";
            statusInfo = "INIT_INFO";
            iconStatus = ICON_FA_EXCLAMATION;
        } else if (engine->GetRunService()->IsRunning()) {
            colorStatus = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
            iconStatus = ICON_FA_PLAY;
        } else if (engine->IsRunnable()) {
            colorStatus = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
            statusInfo = "READY_WAIT_INFO";
            iconStatus = ICON_FA_PLAY;
        }


        ImGui::TextColored(colorStatus, engine->GetName().c_str());

        UI::Core::SetTooltip(myFormat("UUID: {}", engine->deviceUUID), -1);

        if (!CPU::Core::RunManager::Get().isRunning() && ImGui::BeginDragDropTarget()) {
            dropLoadCore(engine);

            ImGui::EndDragDropTarget();
        }

        ImGui::TableNextColumn();
        ImGui::Text("%d", engine->GetOrderPriority());

        ImGui::TableNextColumn();
        ImGui::TextColored(colorStatus, myFormat("{} {}", iconStatus, I18N::Get(status)).c_str());
        Core::SetTooltip(I18N::Get(statusInfo), -1);

        ImGui::TableNextColumn();
        ImGui::Text(myFormat("{}/{}", engine->GetCpuCoreName().first, engine->GetCpuCoreName().second).c_str()); // todo ajout icone lib non chargé !!

        ImGui::TableNextColumn();

        if (engine->IsRunnable()) {
            drawSpeedPopup(engine);

            ImGui::TableNextColumn();

            ImGui::Text("%d", engine->GetRunService()->GetElapsedCycle());
        }

        ImGui::TableSetColumnIndex(7);

        if (engine->IsRunnable()) {
            bool isEnginePlaying = engine->GetRunService()->IsRunning();
            if (ImGui::Button(isEnginePlaying ? ICON_FA_PAUSE : ICON_FA_PLAY)) {
                if (isEnginePlaying) {
                    engine->GetRunService()->Stop();
                } else {
                    engine->GetRunService()->Run();
                }
            }

            ImGui::SameLine();
        } else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
        }

        if (ImGui::Button(ICON_FA_PEN)) {
            m_configEngineModal->OpenEdit(engine);
        }

        ImGui::SameLine();

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            auto deviceUUID = engine->deviceUUID;
            Core::Events::Get().OnEvent<Core::DelayedActionEvent>("ConfigPanel", [deviceUUID](){
                CPU::Core::EngineManager::Get().RemoveEngine(deviceUUID);
            });
        }

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        m_configEngineModal->OnImGuiRender();

        ImGui::PopID();
    }

    void ConfigPanel::drawSpeedPopup(const Ref<CPU::Core::CpuEngine>& engine) {
        if (ImGui::Selectable("##speed", false)) {
            ImGui::OpenPopup("SpeedEdit");
        }

        ImGui::SameLine();

        CPU::Core::RunService& runService = *engine->GetRunService();
        ImGui::Text("%d", runService.getCpuSpeed());

        if (UI::Core::BeginPopup("SpeedEdit")) {
            auto value = (int) runService.getCpuSpeed();

            ImGui::SetKeyboardFocusHere();
            const ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

            if (ImGui::InputInt("##val", &value, 1, 10, flags) && value != runService.getCpuSpeed()) {
                if (value < 1) {
                    value = 1;
                }

                runService.SetCpuSpeed((size_t) value);
                ImGui::CloseCurrentPopup();
            }

            UI::Core::EndPopup();
        }
    }

    void ConfigPanel::OnEvent(Core::AEvent& pEvent) {
        if (pEvent.GetEventType() == UI::Core::EventType::ProjectLoaded) {
            CPU::Core::RunManager::Get().SetSystemClockSpeed(AstraProject::CurrentProject()->getSettings().CpuSpeed);
        }
    }

    void ConfigPanel::drawDevicesTable() {
        ENGINE_PROFILE_FUNCTION();

        addDeviceButton();

        static const ImGuiTableFlags flags =
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("DevTable", 5, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(I18N::Get("NAME"));
            ImGui::TableSetupColumn(I18N::Get("TYPE"));
            ImGui::TableSetupColumn(I18N::Get("STATUS"));
            ImGui::TableSetupColumn(I18N::Get("ADDR_SIZE"));
            ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableHeadersRow();

            int row = 0;
            for (const auto& [deviceUUID, device]: CPU::Core::DevicesManager::Get().GetDevices()) {
                if (device->type == CPU::Core::DeviceType::ENGINE) {
                    continue;
                }

                const auto& physicalDevice = std::dynamic_pointer_cast<CPU::Core::HardwareDevice>(device);

                ImGui::TableNextRow();

                UI::Core::ImGuiUtils::forceColumnMaxSize(5, row, 0);

                ImGui::PushID(row);
                bool res = drawDeviceTableItem(physicalDevice);
                m_configDeviceModal->OnImGuiRender();
                ImGui::PopID();

                row++;

                if (res) {

                    switch (device->type) {
                        using
                        enum Astra::CPU::Core::DeviceType;
                        case KEYBOARD:
                            AstraProject::CurrentProject()->getSettings().keyboardUUID = 0;
                            break;
                        case MOUSE:
                            AstraProject::CurrentProject()->getSettings().mouseUUID = 0;
                            break;
                        case SCREEN:
                            AstraProject::CurrentProject()->getSettings().screenUUID = 0;
                            break;
                        case AUDIO:
                            AstraProject::CurrentProject()->getSettings().audioUUID = 0;
                            break;
                        case SERIAL:
                            AstraProject::CurrentProject()->getSettings().serialUUID = 0;
                            break;
                        default:
                            break;
                    }

                    UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
                    UI::Core::Events::Get().OnEvent<UI::Core::DelayedActionEvent>("ConfigPanel", [deviceUUID]() {
                        CPU::Core::DevicesManager::Get().RemoveDevice(deviceUUID);
                    });
                }
            }

            ImGui::EndTable();
        }
    }

    void ConfigPanel::addDeviceButton() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::SameLine();
        auto addStr = myFormat(ICON_FA_CIRCLE_PLUS "  {}", I18N::Get("ADD"));
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(addStr.c_str()).x + 60));

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (ImGui::Button(addStr.c_str())) {
            ImGui::OpenPopup("AddDevicePopup");
        }

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        if (ImGui::BeginPopup("AddDevicePopup")) {
            ProjectSettings& projectSettings = AstraProject::CurrentProject()->getSettings();

#define DEVICE_ADD(type, name) CPU::Core::DevicesManager::Get().AddDevice(CPU::Core::DeviceCreateData{UUIDGen::New(), type, name})

            if (!projectSettings.audioUUID && ImGui::MenuItem(myFormat(ICON_FA_VOLUME_LOW " {}", I18N::Get("AUDIO")).c_str())) {
                auto dev = DEVICE_ADD(CPU::Core::DeviceType::AUDIO, "Audio");
                projectSettings.audioUUID = dev->deviceUUID;
                UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
            }

            if (!projectSettings.screenUUID && ImGui::MenuItem(myFormat(ICON_FA_DISPLAY " {}", I18N::Get("SCREEN")).c_str())) {
                auto dev = DEVICE_ADD(CPU::Core::DeviceType::SCREEN, "Screen");
                projectSettings.screenUUID = dev->deviceUUID;
                UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
            }

            if (!projectSettings.keyboardUUID && ImGui::MenuItem(myFormat(ICON_FA_KEYBOARD " {}", I18N::Get("KEYBOARD")).c_str())) {
                auto dev = DEVICE_ADD(CPU::Core::DeviceType::KEYBOARD, "Keyboard");
                projectSettings.keyboardUUID = dev->deviceUUID;
                UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
            }

            if (!projectSettings.mouseUUID && ImGui::MenuItem(myFormat(ICON_FA_COMPUTER_MOUSE " {}", I18N::Get("MOUSE")).c_str())) {
                auto dev = DEVICE_ADD(CPU::Core::DeviceType::MOUSE, "Mouse");
                projectSettings.mouseUUID = dev->deviceUUID;
                UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
            }

            if (!projectSettings.serialUUID && ImGui::MenuItem(myFormat(ICON_FA_TERMINAL " {}", I18N::Get("SERIAL")).c_str())) {
                auto dev = DEVICE_ADD(CPU::Core::DeviceType::SERIAL, "Serial");
                projectSettings.serialUUID = dev->deviceUUID;
                UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
            }

            if (ImGui::MenuItem(myFormat(ICON_FA_HARD_DRIVE " {}", I18N::Get("DISK")).c_str())) {
                static int i = 1;
                DEVICE_ADD(CPU::Core::DeviceType::DISK, myFormat("Disk {}", i));
                i++;
            }

            ImGui::EndPopup();
        }

        ImGui::SameLine();
        ImGui::Text(ICON_FA_CIRCLE_QUESTION);
        UI::Core::SetTooltip(I18N::Get("DEVICE_INFO"), -1);
    }

    std::string ConfigPanel::convertDeviceTypeToDisplay(CPU::Core::DeviceType type) {
        for (const auto& [deviceType, deviceTypeName, deviceTypeIcon]: DeviceTypesDisplay) {
            if (deviceType == type) {
                return myFormat("{} {}", deviceTypeIcon, I18N::Get(deviceTypeName));
            }
        }

        return "unknown";
    }

    bool ConfigPanel::drawDeviceTableItem(const Ref<CPU::Core::HardwareDevice>& device) {
        ENGINE_PROFILE_FUNCTION();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::Text(device->GetName().c_str());
        UI::Core::SetTooltip(myFormat("UUID: {}", device->deviceUUID), -1);

        if (!CPU::Core::RunManager::Get().isRunning() && ImGui::BeginDragDropTarget()) {
            dropDeviceFile(device);

            ImGui::EndDragDropTarget();
        }

        ImGui::TableNextColumn();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::TextColored(ImVec4(0.8f, 0.68f, 0.5f, 1.0f), convertDeviceTypeToDisplay(device->type).c_str());

        ImGui::TableNextColumn();
        ImVec4 statusColor;
        switch (device->GetStatus()) {
            case CPU::Core::DeviceStatus::READY:
                statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case CPU::Core::DeviceStatus::BUSY:
                statusColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
                break;
            default:
                statusColor = ImVec4(1.0f, 0.0f, .5f, 1.0f);
        }

        ImGui::TextColored(statusColor, I18N::Get(CPU::Core::Device::convertDeviceStatusToString(device->GetStatus())));

        ImGui::TableNextColumn();
        const auto& addrList = device->GetDeviceAddressList();
        if (addrList.size() == 1) {
            ImGui::Text("%X", addrList[0].first);
        } else {
            ImGui::Text(myFormat("Total {}", addrList.size()).c_str());
            if (ImGui::IsItemHovered()) {
                std::vector<std::string> tmp;
                for (const auto& [addrSize, internalAddr]: addrList) {
                    tmp.emplace_back(myFormat("{},{}", addrSize, internalAddr));
                }
                UI::Core::SetTooltipMultiLine(tmp);
            };
        }

        ImGui::TableNextColumn();

        if (ImGui::Button(ICON_FA_PEN)) {
            m_configDeviceModal->OpenEdit(device);
        }

        ImGui::SameLine();

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            return true;
        }

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        return false;
    }

    void ConfigPanel::dropLoadCore(const Ref<CPU::Core::CpuEngine>& engine) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload")) {
            const auto& metadata = UI::Core::AssetManager::Get().GetMetadata(*(UUID*) payload->Data);

            if (metadata.isValid() && engine->IsCoreValid()) {
                auto tmpDropFile = AstraProject::CurrentProject()->rootDirectory / metadata.FilePath;

                std::ifstream file(tmpDropFile, std::fstream::binary);
                if (file.is_open()) {
                    auto res = engine->GetCore()->LoadFromStream(file);
                    if (res) {
                        UI::Core::Events::Get().OnEvent<Core::NotificationEvent>(
                                AstraMessage::New2(AstraMessageType::Info, I18N::Get("CORE_LOAD_SUCCESS"), engine->GetName()));
                    } else {
                        UI::Core::Events::Get().OnEvent<Core::NotificationEvent>(
                                AstraMessage::New2(AstraMessageType::Warning, I18N::Get("CORE_LOAD_FAILED"), engine->GetName()));
                    }
                    file.close();
                }
            }
        }
    }

    void ConfigPanel::dropDeviceFile(const Ref<CPU::Core::Device>& device) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload")) {
            const auto& metadata = UI::Core::AssetManager::Get().GetMetadata(*(UUID*) payload->Data);

            if (metadata.isValid()) {
                auto tmpDropFile = AstraProject::CurrentProject()->rootDirectory / metadata.FilePath;
                bool isSuccess = false;

                switch (device->type) {
                    case CPU::Core::DeviceType::DISK: {
                        const std::string& fileExtension = metadata.FilePath.extension().string();
                        if (fileExtension == ".dsk") {
                            auto diskDevice = std::dynamic_pointer_cast<CPU::Core::DiskDevice>(device);
                            isSuccess = diskDevice->OpenDisk(metadata.Handle, tmpDropFile);
                        }
                        break;
                    }
                    default:
                        break;
                }

                if (isSuccess) {
                    UI::Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Info, I18N::Get("DEVICE_LOAD_SUCCESS")));
                } else {
                    UI::Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("DEVICE_LOAD_FAILED")));
                }
            }
        }
    }

}