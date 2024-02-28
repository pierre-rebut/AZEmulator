//
// Created by pierr on 31/10/2023.
//
#include "SerialPanel.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "imgui.h"
#include "Commons/Profiling.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TipsText.h"

namespace Astra::UI::App {
    void SerialPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        TipsText::Get().Show("SERIAL_PANEL_INFO");

        if (!m_serialDevice) {
            ImGui::Text(I18N::Get("SERIAL_NOT_CONFIGURED"));
            return;
        }

        ImGui::Text(I18N::Get("CONNECTED_TO"), m_serialDevice->GetName().c_str());
        Core::SetTooltip(myFormat("UUID: {}", m_serialDevice->deviceUUID), -1);

        ImGui::SameLine();
        const auto btnStr = myFormat(ICON_FA_BROOM " {}", I18N::Get("CLEAR"));
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - (ImGui::CalcTextSize(btnStr.c_str()).x + 36));

        if (ImGui::Button(btnStr.c_str())) {
            AstraProject::CurrentProject()->GetData().serialMessages.clear();
        }

        Core::ScopedColour color(ImGuiCol_ChildBg, IM_COL32(10, 10, 10, 180));
        Core::ScopedStyle style(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        ImGui::BeginChild("##serialWindow", ImVec2(0, 0), true);

        m_isWindowFocused = ImGui::IsWindowFocused();

        drawSerialInput();

        ImGui::PushAllowKeyboardFocus(true);
        drawMessageHistory();
        ImGui::PopAllowKeyboardFocus();

        ImGui::EndChild();
    }

    void SerialPanel::drawMessageHistory() {
        const auto& serialMessages = AstraProject::CurrentProject()->GetData().serialMessages;

        ImGuiListClipper clipper;
        clipper.Begin((int) serialMessages.size() + 1);

        while (clipper.Step()) {
            auto it = serialMessages.cbegin();
            std::advance(it, clipper.DisplayStart);
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++, it++) {
                if (it == serialMessages.cend()) {
                    ImGui::Text(myFormat("$ {}", m_currentMessage).c_str());
                    drawCursor();
                } else {
                    const auto& [msg, isUserMsg] = *it;
                    if (isUserMsg) {
                        ImGui::Text("$ ");
                        ImGui::SameLine();
                    }
                    ImGui::Text(msg.c_str());
                }
            }
        }

        clipper.End();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }

    void SerialPanel::drawCursor() {
        if (!m_isWindowFocused) {
            return;
        }

        auto timeEnd = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - m_startTime).count();

        if (elapsed > 600) {
            auto tmpMin = ImGui::GetItemRectMin();
            auto tmpMax = ImGui::GetItemRectMax();

            auto min = ImVec2(tmpMax.x, tmpMin.y);
            auto max = ImVec2(tmpMax.x + 1, tmpMax.y);

            ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255, 255, 255, 255));

            if (elapsed > 1200) {
                m_startTime = timeEnd;
            }
        }
    }

    void SerialPanel::drawSerialInput() {
        ImGuiIO& io = ImGui::GetIO();
        auto shift = io.KeyShift;
        auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
        auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

        if (ImGui::IsWindowFocused()) {
            if (ImGui::IsWindowHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
            }

            io.WantCaptureKeyboard = true;
            io.WantTextInput = true;

            if (!ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
                enterCharacter('\n');
            } else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab))) {
                enterCharacter('\t');
            }

            if (!io.InputQueueCharacters.empty()) {
                for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
                    auto c = io.InputQueueCharacters[i];
                    if (c != 0 && (c == '\n' || c >= 32)) {
                        enterCharacter(c);
                    }
                }
            }
        }
    }

    void SerialPanel::enterCharacter(unsigned short c) {
        if (c == '\n') {
            auto& serialMessages = AstraProject::CurrentProject()->GetData().serialMessages;
            serialMessages.emplace_back(m_currentMessage, true);
            m_serialDevice->SendSerialMessage(m_currentMessage);
            m_currentMessage.clear();
        } else {
            m_currentMessage += c;
        }
    }

    void SerialPanel::OnEvent(Core::AEvent& pEvent) {
        switch (pEvent.GetEventType()) {
            case Core::EventType::ProjectLoaded:
                m_currentMessage.clear();
            case Core::EventType::SettingsUpdated: {
                m_serialDevice = CPU::Core::DevicesManager::Get().GetDeviceOfType<CPU::Core::SerialDevice>(
                        AstraProject::CurrentProject()->getSettings().serialUUID);
                break;
            }
            default:
                break;
        }
    }

    void SerialPanel::OnUpdate(const Core::FrameInfo& pFrameInfo) {
        if (!m_serialDevice) {
            return;
        }

        auto& serialMessages = AstraProject::CurrentProject()->GetData().serialMessages;
        m_serialDevice->UpdateList(serialMessages);

        while (serialMessages.size() > 100) {
            serialMessages.pop_front();
        }
    }
}
