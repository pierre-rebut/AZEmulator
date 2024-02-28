//
// Created by pierr on 02/08/2023.
//
#include "SettingsModal.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/TipsText.h"

namespace Astra::UI::App {
    void SettingsModal::drawPopupContent() {

        ImGui::BeginChild("##globalChild", ImVec2(500, 120), ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::SeparatorText(myFormat(ICON_FA_GLOBE " {}", I18N::Get("GLOBAL_SETTINGS")).c_str());
        drawGlobalSettings();
        ImGui::EndChild();

        ImGui::BeginChild("##projectChild", ImVec2(500, 120), ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::SeparatorText(myFormat(ICON_FA_WRENCH " {}", I18N::Get("PROJECT_SETTINGS")).c_str());
        drawProjectSettings();
        ImGui::EndChild();

        UI::Core::PopupCloseButton();
    }

    void SettingsModal::drawProjectSettings() const {
        AstraProject* project = AstraProject::CurrentProject();
        std::string name = project->getProjectName();
        if (Core::InputText(I18N::Get("PROJECT_NAME"), name)) {
            project->setProjectName(name);
        }

        ProjectSettings& projectSettings = project->getSettings();
        ImGui::Checkbox(I18N::Get("ENABLE_AUTOSAVE"), &projectSettings.autoSaving);
    }

    void SettingsModal::drawGlobalSettings() const {
        if (ImGui::BeginCombo(I18N::Get("LANGUAGE"), I18NImpl::Get().Current().c_str())) {
            for (const auto& lang : I18NImpl::Get().TradList()) {
                if (ImGui::Selectable(lang.c_str(), lang == I18NImpl::Get().Current())) {
                    I18NImpl::Get().LoadTrad(lang);
                }
            }
            ImGui::EndCombo();
        }

        bool tipsEnable = TipsText::Get().IsEnable();
        if (ImGui::Checkbox(I18N::Get("IS_TIPS_ENABLE"), &tipsEnable)) {
            TipsText::Get().SetEnable(tipsEnable);
            LOG_DEBUG("Tips enable: {}", tipsEnable);
        }
    }
}
