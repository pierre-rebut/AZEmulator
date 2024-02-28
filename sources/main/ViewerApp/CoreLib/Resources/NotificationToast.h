//
// Created by pierr on 10/08/2023.
//
#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <sysinfoapi.h>
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "imgui.h"
#include "Commons/format.h"
#include "Commons/AstraMessage.h"

#define NOTIFY_FADE_IN_OUT_TIME            150            // Fade in and out duration
#define NOTIFY_DEFAULT_DISMISS            3000        // Auto dismiss after X ms (default, applied only of no data provided in constructors)
#define NOTIFY_OPACITY                    1.0f        // 0-1 Toast opacity

namespace Astra::UI::Core {

    enum class ToastPhase
    {
        FadeIn,
        Wait,
        FadeOut,
        Expired
    };

    enum class ToastPos
    {
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        Center
    };

    class NotificationToast
    {
    private:
        AstraMessageType type = AstraMessageType::Success;
        std::string title;
        std::string content;
        int dismissTime = NOTIFY_DEFAULT_DISMISS;
        uint64_t creationTime = 0;

    public:

        template<typename... Args>
        void setTitle(const std::string_view& newTitle, Args&& ... args) {
            title = myFormat(newTitle, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void setContent(const std::string_view& newContent, Args&& ... args) {
            content = myFormat(newContent, std::forward<Args>(args)...);
        }

        void setType(AstraMessageType newType) { type = newType; }

        inline const std::string& getTitle() const { return title; };

        inline const char* getDefaultTitle() const {
            switch (type) {
                using
                enum AstraMessageType;
                case Success:
                    return "Success";
                case Warning:
                    return "Warning";
                case Error:
                    return "Error";
                case Info:
                    return "Info";
                default:
                    return nullptr;
            }
        };

        inline const AstraMessageType& getType() { return type; };

        inline ImVec4 getColor() const {
            switch (type) {
                using
                enum AstraMessageType;
                case Success:
                    return {0, 255, 0, 255}; // Green
                case Warning:
                    return {255, 255, 0, 255}; // Yellow
                case Error:
                    return {255, 0, 0, 255}; // Error
                case Info:
                    return {0, 157, 255, 255}; // Blue
                default:
                    return {255, 255, 255, 255}; // White
            }
        }

        inline const char* getIcon() const {
            switch (type) {
                using
                enum AstraMessageType;
                case Success:
                    return ICON_FA_CIRCLE_CHECK;
                case Warning:
                    return ICON_FA_CIRCLE_EXCLAMATION;
                case Error:
                    return ICON_FA_CIRCLE_XMARK;
                case Info:
                    return ICON_FA_CIRCLE_INFO;
                default:
                    return nullptr;
            }
        }

        inline const std::string& getContent() const { return content; };

        inline uint64_t getElapsedTime() const { return GetTickCount() - creationTime; }

        inline ToastPhase getPhase() const {
            using
            enum Astra::UI::Core::ToastPhase;
            const auto elapsed = getElapsedTime();

            if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismissTime + NOTIFY_FADE_IN_OUT_TIME) {
                return Expired;
            } else if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismissTime) {
                return FadeOut;
            } else if (elapsed > NOTIFY_FADE_IN_OUT_TIME) {
                return Wait;
            } else {
                return FadeIn;
            }
        }

        inline float getFadePercent() const {
            const auto phase = getPhase();
            const auto elapsed = getElapsedTime();

            if (phase == ToastPhase::FadeIn) {
                return ((float) elapsed / (float) NOTIFY_FADE_IN_OUT_TIME) * NOTIFY_OPACITY;
            } else if (phase == ToastPhase::FadeOut) {
                return (1.f - (((float) elapsed - (float) NOTIFY_FADE_IN_OUT_TIME - (float) this->dismissTime) / (float) NOTIFY_FADE_IN_OUT_TIME)) *
                       NOTIFY_OPACITY;
            }

            return 1.f * NOTIFY_OPACITY;
        }

        explicit NotificationToast(const Ref<AstraMessage>& msg) : NotificationToast(msg->type) {
            title = msg->getTitle();
            content = msg->getContent();
        }

        explicit NotificationToast(AstraMessageType newType, int newDismissTime = NOTIFY_DEFAULT_DISMISS) : type(newType), dismissTime(newDismissTime) {
            creationTime = GetTickCount();
        }

        template<typename... Args>
        NotificationToast(AstraMessageType newType, std::string newTitle, Args&& ... args)
                : NotificationToast(newType) { content = myFormat(std::move(newTitle), std::forward<Args>(args)...); }

        template<typename... Args>
        NotificationToast(AstraMessageType newType, int dismissTime, std::string newTitle, Args&& ... args)
                : NotificationToast(newType, dismissTime) { content = myFormat(std::move(newTitle), std::forward<Args>(args)...); }
    };

}
