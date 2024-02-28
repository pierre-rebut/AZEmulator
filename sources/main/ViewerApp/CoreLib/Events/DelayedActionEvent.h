//
// Created by pierr on 29/07/2023.
//
#pragma once

#include <functional>
#include <utility>

#include "AEvent.h"
#include "Commons/format.h"

namespace Astra::UI::Core {
    class DelayedActionEvent : public AEvent
    {
    private:
        const std::string m_name;
        const std::function<void()> m_fn;

    public:
        DelayedActionEvent(std::string  mName, const std::function<void()>& mFn) : m_name(std::move(mName)), m_fn(mFn) {}

        EventType GetEventType() const override {
            return EventType::DelayedAction;
        }

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        std::string toString() const override {
            return myFormat("DelayedActionEvent<{}>", m_name);
        }

        void execute() const {
            m_fn();
        }
    };
}
