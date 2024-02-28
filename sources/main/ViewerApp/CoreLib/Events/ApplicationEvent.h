//
// Created by pierr on 13/03/2022.
//

#pragma once

#include "AEvent.h"
#include "Commons/format.h"

// std
#include <sstream>
#include <vector>
#include <filesystem>

namespace Astra::UI::Core {

    class WindowResizeEvent : public AEvent
    {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
                : m_Width(width), m_Height(height) {}

        unsigned int GetWidth() const { return m_Width; }

        unsigned int GetHeight() const { return m_Height; }

        std::string toString() const override {
            return myFormat("WindowResizeEvent: {}, {}", m_Width, m_Height);
        }

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return EventType::WindowResize;
        }

    private:
        unsigned int m_Width;
        unsigned int m_Height;
    };

    class WindowMinimizeEvent : public AEvent
    {
    public:
        explicit WindowMinimizeEvent(bool minimized)
                : m_Minimized(minimized) {}

        bool IsMinimized() const { return m_Minimized; }

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return EventType::WindowMinimize;
        }

    private:
        bool m_Minimized = false;
    };

    class WindowCloseEvent : public AEvent
    {
    public:
        WindowCloseEvent() = default;

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return EventType::WindowClose;
        }

    };

    class WindowFocusEvent : public AEvent
    {
    private:
        int m_focused;

    public:
        explicit WindowFocusEvent(int pFocused) : m_focused{pFocused} {};

        bool isFocus() const { return m_focused; }

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return EventType::WindowFocus;
        }

        std::string toString() const override {
            return myFormat("WindowFocusEvent");
        }
    };

    class WindowDropEvent : public AEvent
    {
    private:
        std::vector<std::filesystem::path> m_paths{};

    public:
        void AddPath(const std::filesystem::path& path) {
            m_paths.emplace_back(path);
        }

        const auto& GetPaths() const {return m_paths;}

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return EventType::WindowDrop;
        }

        std::string toString() const override {
            return myFormat("WindowDropEvent<{}>", m_paths.size());
        }
    };

    class GenericEvent : public AEvent
    {
    private:
        const EventType m_eventType;
    public:
        explicit GenericEvent(EventType eventType) : m_eventType(eventType) {}

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return m_eventType;
        }

        std::string toString() const override {
            return myFormat("GenericEvent<{}>", (int) m_eventType);
        }
    };

}
