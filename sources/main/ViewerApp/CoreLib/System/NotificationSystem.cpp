//
// Created by pierr on 10/08/2023.
//

#include "NotificationSystem.h"
#include "Commons/Profiling.h"
#include "imgui.h"
#include "ViewerApp/CoreLib/Utils.h"

#define NOTIFY_PADDING             20.f
#define NOTIFY_PADDING_MESSAGE_Y   10.f
#define NOTIFY_TOAST_FLAGS         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing


namespace Astra::UI::Core {
    void NotificationSystem::OnImGuiRender() {
        ENGINE_PROFILE_FUNCTION();

        ScopedStyle border(ImGuiStyleVar_WindowRounding, 5.f);
        ScopedColour bgColor(ImGuiCol_WindowBg, ImVec4(80.f / 255.f, 80.f / 255.f, 80.f / 255.f, 100.f / 255.f));

        const auto vpPos = ImGui::GetMainViewport()->Pos;
        const auto vpSize = ImGui::GetMainViewport()->Size;
        const auto toastPos = ImVec2(vpPos.x + vpSize.x - NOTIFY_PADDING, vpPos.y + vpSize.y - NOTIFY_PADDING);
        float height = 0.f;

        std::erase_if(m_notifications, [&height, &toastPos](const auto& toast) {
            if (toast.getPhase() == ToastPhase::Expired) {
                return true;
            }

            drawNotificationToast(toast, toastPos, height);

            return false;
        });
    }

    void NotificationSystem::drawNotificationToast(const NotificationToast& toast, const ImVec2& toastPos, float& height) {
        const auto icon = toast.getIcon();
        const auto& title = toast.getTitle();
        const auto& content = toast.getContent();
        const auto defaultTitle = toast.getDefaultTitle();
        const auto opacity = toast.getFadePercent();

        auto textColor = toast.getColor();
        textColor.w = opacity;

        ImGui::SetNextWindowBgAlpha(opacity);
        ImGui::SetNextWindowPos(ImVec2(toastPos.x, toastPos.y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        if (ImGui::Begin(myFormat("##ToastWindow{}", height).c_str(), nullptr, NOTIFY_TOAST_FLAGS)) {
            ImGui::PushTextWrapPos(toastPos.x / 3.f);

            bool isTitleRendered = false;

            if (icon) {
                ImGui::TextColored(textColor, icon);
                isTitleRendered = true;
            }

            if (!title.empty()) {
                if (icon) {
                    ImGui::SameLine();
                }

                ImGui::Text(title.c_str());
                isTitleRendered = true;
            } else if (defaultTitle) {
                if (icon) {
                    ImGui::SameLine();
                }

                ImGui::Text(defaultTitle);
                isTitleRendered = true;
            }

            if (isTitleRendered && !content.empty()) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
            }

            if (!content.empty()) {
                if (isTitleRendered) {
                    ImGui::Separator();
                }

                ImGui::Text(content.c_str());
            }

            ImGui::PopTextWrapPos();
        }

        height += ImGui::GetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;
        ImGui::End();
    }
}
