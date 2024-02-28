//
// Created by pierr on 23/08/2023.
//

#pragma once

#include <queue>
#include <mutex>

#include "EngineLib/data/Base.h"
#include "Commons/utils/Singleton.h"

#include "ViewerApp/CoreLib/Events/AEvent.h"
#include "ViewerApp/CoreLib/Events/NotificationEvent.h"
#include "NotificationSystem.h"

namespace Astra::UI::Core {

    class Events : public Singleton<Events>
    {
    public:
        static constexpr const char* NAME = "Events";

    private:
        std::mutex m_mtx{};
        std::queue<Ref<AEvent>> m_queueEvent;
        NotificationSystem& m_notify;

    public:
        explicit Events(NotificationSystem& notify);
        Ref<AEvent> pullEvents();

        template<class T = AEvent, typename... Args>
        void OnEvent(Args&& ... pArgs) {
            std::scoped_lock guard(m_mtx);

            m_queueEvent.emplace(CreateRef<T>(std::forward<Args>(pArgs)...));
            if constexpr (std::is_same<T, NotificationEvent>()) {
                m_notify.Notify(NotificationToast(std::forward<Args>(pArgs)...));
            }
        }

        void PushEvent(const Ref<AEvent>& newEvent){
            std::scoped_lock guard(m_mtx);
            m_queueEvent.emplace(newEvent);
        }
    };

} // Astra
