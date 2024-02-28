//
// Created by pierr on 10/08/2023.
//
#pragma once

#include <string>
#include <chrono>

#include "ViewerApp/CoreLib/Resources/NotificationToast.h"

namespace Astra::UI::Core {

    class NotificationSystem
    {
    private:
        std::vector<NotificationToast> m_notifications;

        static void drawNotificationToast(const NotificationToast& toast, const ImVec2& toastPos, float& height);

    public:
        void OnImGuiRender();

        inline void Notify(const NotificationToast& toast) {
            m_notifications.emplace_back(toast);
        }

        inline void Notify(NotificationToast&& toast) {
            m_notifications.emplace_back(std::move(toast));
        }
    };

}
