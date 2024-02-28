//
// Created by pierr on 27/08/2023.
//

#include "MemoryConfigModal.h"

#include "Commons/Profiling.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "Commons/utils/Utils.h"
#include "ViewerApp/CoreLib/ImGuiUtils.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/Custom/Panels/Views/ConfigPanel.h"
#include "CpuEngine/manager/running/RunManager.h"

namespace Astra::UI::App {
    void MemoryConfigModal::OpenEdit(const Ref<CPU::Core::DataBus>& bus) {
        m_dataBus = bus;
        m_selected = nullptr;
        APopup::Open();
    }

    void MemoryConfigModal::drawPopupContent() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::BeginChild("memoryChild", ImVec2(600, 500), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

        connectNewDevice();
        drawConnectedDevicesTable();

        ImGui::EndChild();

        UI::Core::PopupCloseButton();
    }

    void MemoryConfigModal::connectNewDevice() {
        if (ImGui::BeginCombo("##newDevice", m_selected ? m_selected->GetName().c_str() : I18N::Get("NONE"))) {

            for (const auto& [deviceUUID, device] : CPU::Core::DevicesManager::Get().GetDevices()) {
                if (ImGui::Selectable(device->GetName().c_str(), m_selected == device)) {
                    m_selected = device;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (ImGui::Button(myFormat(ICON_FA_LINK " {}", I18N::Get("CONNECT")).c_str()) && m_selected) {
            m_inputModal.OpenInput(I18N::Get("NEW_DEVICE"), {{I18N::Get("ADDRESS"), "0"},
                                                  {I18N::Get("INDEX"), "0"}}, [this](const auto& result) {
                m_dataBus->ConnectDevice(Utils::ParseNumber(result.at(0).second), m_selected, Utils::ParseNumber(result.at(1).second));
                return true;
            });
        }

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        m_inputModal.OnImGuiRender();
    }

    void MemoryConfigModal::drawConnectedDevicesTable() {
        ENGINE_PROFILE_FUNCTION();

        static const ImGuiTableFlags flags =
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                ImGuiTableFlags_BordersOuter |ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("DeviceTable", 5, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(I18N::Get("NAME"));
            ImGui::TableSetupColumn(I18N::Get("TYPE"));
            ImGui::TableSetupColumn(I18N::Get("INDEX"));
            ImGui::TableSetupColumn(I18N::Get("ADDRESS"));
            ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableHeadersRow();

            int row = 0;
            for (const auto& connectedDevice : m_dataBus->GetLinkedDevices()) {
                ImGui::TableNextRow();
                UI::Core::ImGuiUtils::forceColumnMaxSize(5, row, 0);

                ImGui::PushID(row);
                bool res = drawDeviceTableItem(connectedDevice);
                ImGui::PopID();

                row++;

                if (res) {
                    UI::Core::Events::Get().OnEvent<UI::Core::DelayedActionEvent>("MemoryConfigModal", [this, &connectedDevice](){
                        m_dataBus->DisconnectDevice(connectedDevice);
                    });
                }
            };

            ImGui::EndTable();
        }
    }

    bool MemoryConfigModal::drawDeviceTableItem(const CPU::Core::ConnectedDevice& connectedDevice) {
        ENGINE_PROFILE_FUNCTION();

        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::Text(connectedDevice.device->GetName().c_str());
        UI::Core::SetTooltip(myFormat("UUID: {}", connectedDevice.device->deviceUUID), -1);

        ImGui::TableNextColumn();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), ConfigPanel::convertDeviceTypeToDisplay(connectedDevice.device->type).c_str());

        ImGui::TableNextColumn();
        ImGui::Text("%d", connectedDevice.index);

        ImGui::TableNextColumn();

        if (connectedDevice.addressLow != connectedDevice.addressHigh) {
            ImGui::Text("0x%08x to 0x%08x", connectedDevice.addressLow, connectedDevice.addressHigh);
        } else {
            ImGui::Text("0x%08x", connectedDevice.addressLow);
        }

        ImGui::TableNextColumn();

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        auto ret = ImGui::Button(ICON_FA_LINK_SLASH);

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        return ret;
    }

    void MemoryConfigModal::Reset() {
        m_selected = nullptr;
        m_dataBus = nullptr;
    }
} // Astra