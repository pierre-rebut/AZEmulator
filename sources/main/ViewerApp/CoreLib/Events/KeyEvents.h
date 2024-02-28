//
// Created by pierr on 19/10/2023.
//
#pragma once

#include "AEvent.h"
#include "EngineLib/data/KeyCodes.h"
#include "Commons/format.h"

namespace Astra::UI::Core {

    class KeyEvent : public AEvent
    {
    protected:

    public:
        const KeyCode keyCode;

        explicit KeyEvent(KeyCode keycode) : keyCode(keycode) {}

        EventCategory GetCategoryFlag() const override { return EventCategory::EventCategoryApplication; }
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        const int repeatCount;

        explicit KeyPressedEvent(KeyCode keycode, int mRepeatCount = 0) : KeyEvent(keycode), repeatCount(mRepeatCount) {}

        EventType GetEventType() const override { return EventType::KeyPressed; }

        std::string toString() const override {
            return myFormat("KeyPressedEvent: {} (count {})", (int) keyCode, repeatCount);
        }
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        using KeyEvent::KeyEvent;

        EventType GetEventType() const override { return EventType::KeyReleased; }

        std::string toString() const override {
            return myFormat("KeyReleasedEvent: {}", (int) keyCode);
        }
    };
}
