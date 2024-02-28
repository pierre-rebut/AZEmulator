//
// Created by pierr on 23/08/2023.
//

#pragma once

#include "AEvent.h"
#include "Commons/AstraMessage.h"
#include "EngineLib/data/Base.h"

namespace Astra::UI::Core {

    class NotificationEvent : public AEvent
    {
    private:
        Ref<AstraMessage> m_msg;
    public:
        explicit NotificationEvent(const Ref<AstraMessage>& msg) : m_msg(msg) {}

        const Ref<AstraMessage>& GetMessage() const {return m_msg;}

        std::string toString() const override {
            return myFormat("NotificationEvent<{}>", m_msg->getTitle());
        }

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryApplication;
        }

        EventType GetEventType() const override {
            return EventType::Notification;
        }
    };

} // Astra
