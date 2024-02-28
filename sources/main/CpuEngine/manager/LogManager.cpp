//
// Created by pierr on 08/08/2023.
//
#include "LogManager.h"

namespace Astra::CPU::Core {
    void LogManager::Notify(const Ref<AstraMessage>& newMessage) {
        std::scoped_lock guard(m_mutex);
        m_queue.push(newMessage);
    }

    void LogManager::RetrieveMessages(std::list<Ref<AstraMessage>>& pMsg) {
        std::scoped_lock guard(m_mutex);

        while (!m_queue.empty()) {
            pMsg.emplace_back(m_queue.front());
            m_queue.pop();
        }
    }
}
