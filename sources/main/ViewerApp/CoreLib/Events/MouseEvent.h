//
// Created by pierr on 06/02/2024.
//
#pragma once

#include "AEvent.h"
#include "EngineLib/data/KeyCodes.h"
#include "Commons/format.h"

namespace Astra::UI::Core {

    class MouseMovedEvent : public AEvent
    {
    public:
        const float x;
        const float y;

        MouseMovedEvent(float x, float y) : x(x), y(y) {}

        EventCategory GetCategoryFlag() const override { return EventCategory::EventCategoryApplication; }
        EventType GetEventType() const override { return EventType::MouseMoved; }

        std::string toString() const override {
            return myFormat("MouseMovedEvent: x = {}, y = {}", x, y);
        }
    };

    class MouseScrolledEvent : public AEvent
    {
    public:
        const float xOffset;
        const float yOffset;

        MouseScrolledEvent(float x, float y) : xOffset(x), yOffset(y) {}

        EventCategory GetCategoryFlag() const override { return EventCategory::EventCategoryApplication; }
        EventType GetEventType() const override { return EventType::MouseScrolled; }

        std::string toString() const override {
            return myFormat("MouseScrolledEvent: x = {}, y = {}", xOffset, yOffset);
        }
    };

    class MouseButtonEvent : public AEvent
    {
    public:
        const MouseCode mouseCode;

        explicit MouseButtonEvent(MouseCode mCode) : mouseCode(mCode) {}

        EventCategory GetCategoryFlag() const override { return EventCategory::EventCategoryApplication; }
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        using MouseButtonEvent::MouseButtonEvent;

        EventType GetEventType() const override { return EventType::MouseButtonPressed; }

        std::string toString() const override {
            return myFormat("MouseButtonPressedEvent: {}", (int) mouseCode);
        }
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        using MouseButtonEvent::MouseButtonEvent;

        EventType GetEventType() const override { return EventType::MouseButtonReleased; }

        std::string toString() const override {
            return myFormat("MouseButtonReleasedEvent: {}", (int) mouseCode);
        }
    };

}
