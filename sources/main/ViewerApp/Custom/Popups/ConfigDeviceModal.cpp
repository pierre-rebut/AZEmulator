//
// Created by pierr on 16/01/2024.
//
#include "ConfigDeviceModal.h"

#include "imgui.h"

#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/Events/NotificationEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/CoreLib/Assets/AssetMetadata.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "ViewerApp/CoreLib/Utils.h"

#include "Commons/Profiling.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "CpuEngine/engine/hardwareDevices/impl/AudioDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/DiskDevice.h"

namespace Astra::UI::App {

    void ConfigDeviceModal::OpenEdit(const Ref<CPU::Core::HardwareDevice>& device) {
        m_device = device;
        Core::APopup::Open();
    }

    void ConfigDeviceModal::drawPopupContent() {
        ENGINE_PROFILE_FUNCTION();

        UI::Core::ScopedStyle style(ImGuiStyleVar_WindowPadding, ImVec2(15, 10));
        ImGui::BeginChild("##child", ImVec2(800, 500), ImGuiWindowFlags_AlwaysUseWindowPadding);

        drawNameInput();

        bool isCpuRunning = CPU::Core::RunManager::Get().isRunning();

        if (isCpuRunning) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        ImGui::BeginTabBar("##tabConfig", ImGuiTabBarFlags_FittingPolicyScroll);
        drawDeviceConfig();
        ImGui::EndTabBar();

        if (isCpuRunning) {
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
        }

        ImGui::EndChild();
        UI::Core::PopupCloseButton();
    }

    void ConfigDeviceModal::drawNameInput() {
        std::string name = m_device->GetName();
        if (Core::InputText(I18N::Get("DEVICE_NAME"), name)) {
            m_device->SetName(name);
        }
    }

    void ConfigDeviceModal::drawDeviceConfig() {
        switch (m_device->type) {
            case CPU::Core::DeviceType::AUDIO:
                if (ImGui::BeginTabItem(myFormat(ICON_FA_VOLUME_LOW " {}", I18N::Get("AUDIO")).c_str())) {
                    drawAudioDeviceConfig();
                    ImGui::EndTabItem();
                }
                break;
            case CPU::Core::DeviceType::SCREEN:
                if (ImGui::BeginTabItem(myFormat(ICON_FA_DISPLAY " {}", I18N::Get("SCREEN")).c_str())) {
                    drawScreenDeviceConfig();
                    ImGui::EndTabItem();
                }
                break;
            case CPU::Core::DeviceType::DISK:
                if (ImGui::BeginTabItem(myFormat(ICON_FA_HARD_DRIVE " {}", I18N::Get("DISK")).c_str())) {
                    drawDiskDeviceConfig();
                    ImGui::EndTabItem();
                }
                break;
            default:
                break;
        }

    }

    void ConfigDeviceModal::drawDiskDeviceConfig() {

        const auto& diskDevice = std::dynamic_pointer_cast<CPU::Core::DiskDevice>(m_device);
        const auto& metaData = Core::AssetManager::Get().GetMetadata(diskDevice->GetCurrentDiskId());
        auto filepath = metaData.FilePath.string();

        ImGui::PushItemWidth(-180);
        ImGui::InputText(I18N::Get("DISK"), (char*) filepath.c_str(), filepath.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN)) {
            ImGui::OpenPopup("diskLoad");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_EJECT)) {
            diskDevice->CloseDisk();
        }
        ImGui::PopItemWidth();

        bool isReadOnly = diskDevice->IsReadOnly();
        if (Core::Toggle(I18N::Get("READ_ONLY"), isReadOnly)) {
            diskDevice->SetReadOnly(isReadOnly);
        }

        ImGui::NewLine();
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("DISK_DEVICE_INFO")).c_str());
        ImGui::PopTextWrapPos();

        if (ImGui::BeginPopup("diskLoad")) {

            for (const auto& [assetUUID, assetMetadata]: Core::AssetManager::Get().GetAssetsList()) {
                if (!assetMetadata.FilePath.filename().string().ends_with("dsk")) {
                    continue;
                }

                if (ImGui::MenuItem(assetMetadata.FilePath.string().c_str())) {
                    diskDevice->OpenDisk(assetUUID, Core::AssetMetadata::GetFileSystemPath(assetMetadata));
                }
            }

            ImGui::EndPopup();
        }
    }

    void ConfigDeviceModal::drawScreenDeviceConfig() {
        const auto& screenDevice = std::dynamic_pointer_cast<CPU::Core::ScreenDevice>(m_device);

        auto width = static_cast<int>(screenDevice->getWidth());
        auto height = static_cast<int>(screenDevice->getHeight());

        if (ImGui::InputInt(I18N::Get("WIDTH"), &width, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue)) {
            applyNewScreenSettings(screenDevice, width, height);
        }

        if (ImGui::InputInt(I18N::Get("HEIGHT"), &height, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue)) {
            applyNewScreenSettings(screenDevice, width, height);
        }

        ImGui::Spacing();
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("SCREEN_DEVICE_INFO")).c_str());
        ImGui::PopTextWrapPos();
    }

    void ConfigDeviceModal::applyNewScreenSettings(const Ref<CPU::Core::ScreenDevice>& screenDevice, int width, int height) {
        if (screenDevice->SetScreenSize(width, height)) {
            UI::Core::Events::Get().OnEvent<Core::GenericEvent>(Core::EventType::SettingsUpdated);
        } else {
            UI::Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("DEVICE_UPDATE_FAILED")));
        }
    }

    void ConfigDeviceModal::drawAudioDeviceConfig() {
        const auto& audioDevice = std::dynamic_pointer_cast<CPU::Core::AudioDevice>(m_device);

        auto masterVolume = audioDevice->GetMasterVolume();
        if (ImGui::InputFloat(myFormat(ICON_FA_VOLUME_LOW " {}", I18N::Get("MASTER_VOLUME")).c_str(), &masterVolume, 0.01f, 0.1f) && masterVolume >= 0 &&
            masterVolume <= 1) {
            audioDevice->SetMasterVolume(masterVolume);
            UI::Core::Events::Get().OnEvent<UI::Core::GenericEvent>(UI::Core::EventType::SettingsUpdated);
        }

        ImGui::Spacing();
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled(myFormat(ICON_FA_CIRCLE_INFO " {}", I18N::Get("AUDIO_DEVICE_INFO")).c_str());
        ImGui::PopTextWrapPos();
    }
}
