//
// Created by pierr on 31/07/2023.
//
#include "ConfigEngineModal.h"

#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"

#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "ViewerApp/CoreLib/Events/NotificationEvent.h"
#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/CoreLib/ImGuiUtils.h"

#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/AsyncJob.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"

namespace Astra::UI::App {

    void ConfigEngineModal::OpenEdit(const Ref<CPU::Core::CpuEngine>& engine) {
        m_engine = engine;
        APopup::Open();
    }

    void ConfigEngineModal::drawPopupContent() {
        ENGINE_PROFILE_FUNCTION();

        UI::Core::ScopedStyle style(ImGuiStyleVar_WindowPadding, ImVec2(15, 10));
        ImGui::BeginChild("##child", ImVec2(800, 500), ImGuiWindowFlags_AlwaysUseWindowPadding);

        drawNameInput();

        ImGui::BeginTabBar("##tabConfig", ImGuiTabBarFlags_FittingPolicyScroll);

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        drawCpuMain(isCpuRunning);
        drawCpuBinding(isCpuRunning);
        drawInterruptionConfig(isCpuRunning);
        drawHardParamConfig(isCpuRunning);

        ImGui::EndTabBar();

        ImGui::EndChild();
        UI::Core::PopupCloseButton();
    }

    void ConfigEngineModal::drawNameInput() {
        std::string name = m_engine->GetName();
        if (Core::InputText(I18N::Get("ENGINE_NAME"), name)) {
            m_engine->SetName(name);
        }

        if (m_engine->IsRunnable()) {
            bool isAutoStart = m_engine->IsAutoStart();
            if (UI::Core::Toggle(I18N::Get("AUTO_START"), isAutoStart, ImGuiToggleFlags_Animated)) {
                m_engine->SetAutoStart(isAutoStart);
            }

            ImGui::SameLine();
            ImGui::Text(ICON_FA_CIRCLE_QUESTION);
            Core::SetTooltip(I18N::Get("AUTO_START_INFO"), -1);
        }

        int orderPriority = m_engine->GetOrderPriority();
        if (ImGui::InputInt(I18N::Get("ORDER_PRIORITY"), &orderPriority, 1, 1) && orderPriority >= 0) {
            m_engine->SetOrderPriority(orderPriority);
            UI::Core::AsyncJob::Get().PushTask([](){

                static std::mutex mtx{};
                std::scoped_lock guard(mtx);

                CPU::Core::EngineManager::Get().UpdateOrderPriorityList();
                CPU::Core::RunManager::Get().UpdateOrderPriorityList();
            });
        }

        ImGui::SameLine();
        ImGui::Text(ICON_FA_CIRCLE_QUESTION);
        UI::Core::SetTooltip(I18N::Get("ORDER_PRIORITY_INFO"), -1);
    }

    void ConfigEngineModal::drawCpuMain(bool isCpuRunning) const {
        if (ImGui::BeginTabItem(myFormat(ICON_FA_BOOK_OPEN " {}", I18N::Get("LIB_CONFIG")).c_str())) {
            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("INFO_LOAD_CORE")).c_str());
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            ImGui::Text(myFormat("{} : {}/{}", I18N::Get("CORE_LIB_NAME"), m_engine->GetCpuCoreName().first, m_engine->GetCpuCoreName().second).c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 90);

            if (ImGui::Button(myFormat(ICON_FA_PEN " {}", I18N::Get("EDIT")).c_str())) {
                ImGui::OpenPopup("LibDirPopup");
            }

            const auto& coreLibManager = CPU::Core::CoreLibManager::Get();

            if (ImGui::BeginPopup("LibDirPopup")) {
                const auto& [currentEngineDir, currentEngineLib] = m_engine->GetCpuCoreName();

                for (const auto& [dirName, dirEntry]: coreLibManager.GetCoreLibs()) {

                    if (ImGui::BeginMenu(dirName.c_str())) {
                        bool isSameDir = (currentEngineDir == dirName);

                        for (const auto& [libName, coreLib]: dirEntry->libs) {
                            if (ImGui::MenuItem(libName.c_str(), nullptr, isSameDir && currentEngineLib == libName)) {
                                coreLibManager.SetAndLoadEngineCoreLib(dirName, libName, m_engine);
                                break;
                            }
                        }

                        ImGui::EndMenu();
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_TRASH_CAN)) {
                coreLibManager.UnsetAndUnloadEngineCoreLib(m_engine);
            }

            if (isCpuRunning) {
                ImGui::PopStyleVar();
                ImGui::PopItemFlag();
            }

            ImGui::EndTabItem();
        }
    }

    void ConfigEngineModal::drawCpuBinding(bool isCpuRunning) const {
        const auto& devicesBind = m_engine->GetEntities().GetDevices();
        if (devicesBind.empty()) {
            return;
        }

        if (ImGui::BeginTabItem(myFormat(ICON_FA_NETWORK_WIRED " {}", I18N::Get("DEVICE_BIND")).c_str())) {

            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("DEVICE_BIND_INFO")).c_str());
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            const auto& busManager = CPU::Core::DataBusManager::Get();
            const auto& devManager = CPU::Core::DevicesManager::Get();

            static const ImGuiTableFlags flags =
                    ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp;

            if (ImGui::BeginTable("DeviceInterConnections", 3, flags)) {

                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(I18N::Get("DEVICE"));
                ImGui::TableSetupColumn(I18N::Get("VALUE"));
                ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 160);
                ImGui::TableHeadersRow();

                int row = 0;
                for (const auto& [deviceName, dev]: devicesBind) {
                    ImGui::PushID(row);

                    ImGui::TableNextRow();
                    UI::Core::ImGuiUtils::forceColumnMaxSize(3, row, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text(deviceName.c_str());

                    ImGui::TableNextColumn();

                    if (const auto bindDevice = dev->GetValue()) {
                        ImGui::Text(myFormat("{} (UUID: {})", bindDevice->deviceName, bindDevice->deviceUUID).c_str());
                    } else {
                        ImGui::Text(I18N::Get("NONE"));
                    }

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, Core::Colors::compliment);
                    if (ImGui::Button(myFormat(ICON_FA_PEN " {}", I18N::Get("EDIT")).c_str())) {
                        ImGui::OpenPopup("DevPopup");
                    }
                    ImGui::PopStyleColor();

                    if (ImGui::BeginPopup("DevPopup")) {
                        drawDevConfigPopup(busManager, devManager, dev.get());

                        ImGui::EndPopup();
                    }

                    ImGui::SameLine();

                    ImGui::AlignTextToFramePadding();
                    if (UI::Core::Widgets::IconButton(ICON_FA_TRASH_CAN)) {
                        dev->SetValue(nullptr);
                    }

                    ImGui::PopID();
                    row++;
                }

                ImGui::EndTable();
            }

            if (isCpuRunning) {
                ImGui::PopStyleVar();
                ImGui::PopItemFlag();
            }

            ImGui::EndTabItem();
        }
    }

    void ConfigEngineModal::drawDevConfigPopup(const CPU::Core::DataBusManager& busManager, const CPU::Core::DevicesManager& devManager,
                                               const CPU::Core::DeviceValue* dev) {
        auto devValue = dev->GetValue();
        if (ImGui::BeginMenu(I18N::Get("MENU_MEMORY"))) {
            for (const auto& [busUUID, busInfo]: busManager.GetDataBuses()) {
                if (ImGui::MenuItem(busInfo->GetName().c_str(), nullptr, devValue && devValue->deviceUUID == busUUID)) {
                    dev->SetValue(busInfo);
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(I18N::Get("MENU_DEVICES"))) {
            for (const auto& [devUUID, devInfo]: devManager.GetDevices()) {
                if (ImGui::MenuItem(devInfo->GetName().c_str(), nullptr, devValue && devValue->deviceUUID == devUUID)) {
                    dev->SetValue(devInfo);
                    break;
                }
            }
            ImGui::EndMenu();
        }
    }

    void ConfigEngineModal::drawInterruptionConfig(bool isCpuRunning) {
        if (!m_engine->IsCoreValid() || !m_engine->GetCoreConfigInfo()->allowDeviceConnection) {
            return;
        }

        if (ImGui::BeginTabItem(myFormat(ICON_FA_LINK " {}", I18N::Get("INTERRUPT")).c_str())) {

            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("INTERRUPT_INFO")).c_str());
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            drawDeviceInterruptAdd();
            drawDeviceInterruptConnections();

            if (isCpuRunning) {
                ImGui::PopStyleVar();
                ImGui::PopItemFlag();
            }

            ImGui::EndTabItem();
        }
    }

    void ConfigEngineModal::drawDeviceInterruptAdd() {
        const auto selectedName = (m_tmpDeviceConnection ? m_tmpDeviceConnection->GetName() : I18N::Get("NONE"));

        if (ImGui::BeginCombo("##AddDevice", selectedName.c_str())) {

            for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                if (ImGui::Selectable(engine->GetName().c_str(), m_tmpDeviceConnection && m_tmpDeviceConnection->deviceUUID == engineUUID)) {
                    m_tmpDeviceConnection = engine;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if (ImGui::Button(myFormat(ICON_FA_CIRCLE_PLUS " {}", I18N::Get("ADD")).c_str()) && m_tmpDeviceConnection) {
            bool res = m_engine->ConnectCpuInterrupt(m_tmpDeviceConnection);
            m_tmpDeviceConnection = nullptr;
            if (!res) {
                UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("CPU_CONNECTION_FAILED")));
            }
        }
    }

    void ConfigEngineModal::drawDeviceInterruptConnections() {
        static const ImGuiTableFlags flags =
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("DeviceInterConnections", 2, flags)) {

            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(I18N::Get("ENGINE"));
            ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableHeadersRow();

            int row = 0;
            for (const auto& connectedEngine: m_engine->GetInterruptServices()) {
                ImGui::PushID(row);

                ImGui::TableNextRow();
                UI::Core::ImGuiUtils::forceColumnMaxSize(2, row, 0);

                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", connectedEngine->GetName().c_str());
                UI::Core::SetTooltip(myFormat("UUID: {}", connectedEngine->deviceUUID), -1);

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button(ICON_FA_TRASH_CAN)) {
                    UI::Core::Events::Get().OnEvent<UI::Core::DelayedActionEvent>("ConfigEngineModal", [this, connectedEngine](){
                        m_engine->DisconnectCpuInterrupt(connectedEngine);
                    });
                }

                ImGui::PopID();
                row++;
            }

            ImGui::EndTable();
        }
    }

    void ConfigEngineModal::drawHardParamConfig(bool isCpuRunning) {
        if (!m_engine->GetCoreConfigInfo() || m_engine->GetCoreConfigInfo()->hardParameters.empty()) {
            return;
        }

        if (ImGui::BeginTabItem(myFormat(ICON_FA_FLASK " {}", I18N::Get("HARD_PARAMETERS")).c_str())) {

            if (isCpuRunning) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }

            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("HARD_PARAMETERS_INFO")).c_str());
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            static const ImGuiTableFlags flags =
                    ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedSame;

            if (ImGui::BeginTable("DeviceHardParam", 2, flags, ImVec2(0, -40))) {

                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(I18N::Get("NAME"));
                ImGui::TableSetupColumn(I18N::Get("VALUE"));
                ImGui::TableHeadersRow();

                int row = 0;
                auto& hardParamVals = m_engine->GetHardParameters();

                for (const auto& paramName: m_engine->GetCoreConfigInfo()->hardParameters) {
                    ImGui::PushID(row);

                    auto& paramVal = hardParamVals.at(row);

                    ImGui::TableNextRow();
                    UI::Core::ImGuiUtils::forceColumnMaxSize(2, row, 0);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text(paramName);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(200);
                    ImGui::InputInt("##val", &paramVal, 1);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_TRASH_CAN)) {
                        paramVal = 0;
                    }

                    ImGui::PopID();
                    row++;
                }

                ImGui::EndTable();
            }

            const float contentRegionWidth = ImGui::GetContentRegionAvail().x;
            auto btnStr = myFormat(ICON_FA_CHECK " {}", I18N::Get("APPLY"));
            const float buttonWidth = ImGui::CalcTextSize(btnStr.c_str()).x + 36;

            UI::Core::ShiftCursorX(((contentRegionWidth - buttonWidth) / 2.0f) - ImGui::GetStyle().ItemSpacing.x);

            if (ImGui::Button(btnStr.c_str())) {
                updateHardParameters();
            }

            if (isCpuRunning) {
                ImGui::PopStyleVar();
                ImGui::PopItemFlag();
            }

            ImGui::EndTabItem();
        }
    }

    void ConfigEngineModal::updateHardParameters() {
        if (!m_engine->UpdateHardParameters()) {
            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("PARAM_UPDATE_FAILED"), m_engine->deviceUUID));
            return;
        }

        if (CPU::Core::DataBusManager::Get().RefreshDeviceConnections(m_engine)) {
            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("REFRESH_CONNECTION_FAILED"), m_engine->deviceUUID));
        }
    }
}
