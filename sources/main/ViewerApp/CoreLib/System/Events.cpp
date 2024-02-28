//
// Created by pierr on 23/08/2023.
//

#include "Events.h"
#include "Commons/Profiling.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "Commons/Log.h"

namespace Astra::UI::Core {
    Events::Events(NotificationSystem& notify) : m_notify(notify) {
    }

    Ref<AEvent> Events::pullEvents() {
        std::scoped_lock guard(m_mtx);

        if (m_queueEvent.empty()) {
            return nullptr;
        }

        auto evt = m_queueEvent.front();
        m_queueEvent.pop();
        return evt;
    }
} // Astra