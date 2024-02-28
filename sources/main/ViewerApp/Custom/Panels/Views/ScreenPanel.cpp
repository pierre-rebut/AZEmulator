//
// Created by pierr on 18/03/2023.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "ScreenPanel.h"

#include "Commons/Profiling.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "CpuEngine/manager/running/RunManager.h"

#include "CpuEngine/manager/devices/DevicesManager.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/Events/KeyEvents.h"
#include "ViewerApp/CoreLib/Events/MouseEvent.h"

namespace Astra::UI::App {
    ScreenPanel::ScreenPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {
    }

    void ScreenPanel::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) {
        if (!m_isOpen) { return; }

        if (ImGui::Begin(NAME, nullptr,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {

            if (m_screenTexture) {
                drawPanelContent();
            }
        }

        ImGui::End();
    }

    void ScreenPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        const ImVec2 contentAvail = ImGui::GetContentRegionAvail();
        const ImVec2 screenMin = ImGui::GetCursorScreenPos();

        m_isFocus = ImGui::IsWindowFocused();
        m_screenPos = ImGui::GetWindowPos() - ImGui::GetWindowViewport()->Pos + ImVec2(0, 35);
        m_screenSize = ImGui::GetWindowSize();

        if (m_isFocus) {
            const auto mousePos = ImGui::GetMousePos();
            if (mousePos.x >= m_screenPos.x && mousePos.y >= m_screenPos.y) {
                const auto maxPos = m_screenSize + m_screenPos;
                if (mousePos.x <= maxPos.x && mousePos.y <= maxPos.y) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
                }
            }

            auto* drawList = ImGui::GetWindowDrawList();
            const auto screenMax = ImVec2(contentAvail.x + screenMin.x, contentAvail.y + screenMin.y);
            drawList->AddRect(screenMin, screenMax, UI::Core::Colors::compliment, 3, 0, 3);
        }

        m_screenTexture->updateTexture(m_screenDevice->getData());

        ImGui::SetCursorScreenPos(ImVec2(screenMin.x + 3, screenMin.y + 3));
        const auto imageSize = ImVec2(contentAvail.x - 6, contentAvail.y - 6);
        ImGui::Image(m_screenTexture->textureId(), imageSize);
    }

    void ScreenPanel::OnEvent(UI::Core::AEvent& pEvent) {
        const auto& runManager = CPU::Core::RunManager::Get();

        switch (pEvent.GetEventType()) {
            case UI::Core::EventType::SettingsUpdated:
            case UI::Core::EventType::ProjectLoaded: {
                const auto& deviceManager = CPU::Core::DevicesManager::Get();

                m_sound = nullptr;
                m_screenTexture = nullptr;

                const ProjectSettings& projectSettings = AstraProject::CurrentProject()->getSettings();

                m_audioDevice = deviceManager.GetDeviceOfType<CPU::Core::AudioDevice>(projectSettings.audioUUID);
                if (m_audioDevice) {
                    m_sound = Core::Sound::CreateSound(44100);
                    m_sound->SetDataCallback(std::bind_front(&CPU::Core::AudioDevice::PopSampleQueue, m_audioDevice));
                    if (!m_sound->SetMasterVolume(m_audioDevice->GetMasterVolume())) {
                        UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(
                                AstraMessage::New2(AstraMessageType::Warning, I18N::Get("DEVICE_UPDATE_FAILED")));
                    }
                }

                m_keyboardDevice = deviceManager.GetDeviceOfType<CPU::Core::KeyboardDevice>(projectSettings.keyboardUUID);
                m_mouseDevice = deviceManager.GetDeviceOfType<CPU::Core::MouseDevice>(projectSettings.mouseUUID);

                m_screenDevice = deviceManager.GetDeviceOfType<CPU::Core::ScreenDevice>(projectSettings.screenUUID);
                if (m_screenDevice) {
                    m_screenTexture = CreateScope<UI::Core::Texture>(m_screenDevice->getData(), m_screenDevice->getWidth(), m_screenDevice->getHeight());
                }

                break;
            }
            case UI::Core::EventType::KeyPressed: {
                const auto& keyEvent = (const UI::Core::KeyPressedEvent&) pEvent;
                if (m_keyboardDevice && m_isFocus && runManager.isRunning()) {
                    m_keyboardDevice->SetControllerKey(keyEvent.keyCode);
                }

                pEvent.Handled = true;
                break;
            }
            case UI::Core::EventType::KeyReleased: {
                const auto& keyEvent = (const UI::Core::KeyReleasedEvent&) pEvent;
                if (m_keyboardDevice && m_isFocus && runManager.isRunning()) {
                    m_keyboardDevice->UnsetControllerKey(keyEvent.keyCode);
                }

                pEvent.Handled = true;
                break;
            }
            case UI::Core::EventType::MouseMoved: {
                const auto& mouseEvent = (const UI::Core::MouseMovedEvent&) pEvent;
                if (m_mouseDevice && m_isFocus && runManager.isRunning() && mouseEvent.x >= m_screenPos.x && mouseEvent.y >= m_screenPos.y) {
                    const auto maxPos = m_screenSize + m_screenPos;
                    if (mouseEvent.x < maxPos.x && mouseEvent.y < maxPos.y) {

                        const auto x = mouseEvent.x - m_screenPos.x - (m_screenSize.x / 2);
                        const auto y = mouseEvent.y - m_screenPos.y - (m_screenSize.y / 2);

                        m_mouseDevice->SetMove(x, y);
                    }
                }

                pEvent.Handled = true;
                break;
            }
            case UI::Core::EventType::MouseScrolled: {
                const auto& mouseEvent = (const UI::Core::MouseScrolledEvent&) pEvent;
                if (m_mouseDevice && m_isFocus && runManager.isRunning()) {
                    m_mouseDevice->SetWheel(mouseEvent.yOffset);
                }

                pEvent.Handled = true;
                break;
            }
            case UI::Core::EventType::MouseButtonPressed: {
                const auto& mouseEvent = (const UI::Core::MouseButtonPressedEvent&) pEvent;
                if (m_mouseDevice && m_isFocus && runManager.isRunning()) {
                    m_mouseDevice->SetButtonDown(mouseEvent.mouseCode);
                }

                pEvent.Handled = true;
                break;
            }
            case UI::Core::EventType::MouseButtonReleased: {
                const auto& mouseEvent = (const UI::Core::MouseButtonReleasedEvent&) pEvent;
                if (m_mouseDevice && m_isFocus && runManager.isRunning()) {
                    m_mouseDevice->SetButtonUp(mouseEvent.mouseCode);
                }

                pEvent.Handled = true;
                break;
            }
            case UI::Core::EventType::CpuRun:
                if (m_sound) {
                    m_sound->SetPauseAudio(false);
                }
                break;
            case UI::Core::EventType::CpuStop:
                if (m_sound) {
                    m_sound->SetPauseAudio(true);
                }
                break;
            default:
                break;
        }
    }

    void ScreenPanel::OnUpdate(const Core::FrameInfo& pFrameInfo) {
        if (m_mouseDevice) {
            m_mouseDevice->Update();
        }
    }
}
