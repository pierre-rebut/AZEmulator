//
// Created by pierr on 31/10/2023.
//
#pragma once

#include "Commons/utils/Singleton.h"

#include <functional>
#include <thread>
#include <mutex>
#include <list>

namespace Astra::UI::Core {

    class AsyncJob : public Singleton<AsyncJob>
    {
    public:
        static constexpr const char* NAME = "AsyncJob";

    private:
        std::jthread m_thread;
        std::mutex m_mtx;
        std::list<std::function<void()>> m_jobsList{};

    public:
        AsyncJob();
        ~AsyncJob() override;

        template<class T, typename... Args> requires std::is_member_function_pointer_v<T>
        void PushTask(T&& fn, Args&&... pArgs) {
            std::scoped_lock guard(m_mtx);
            m_jobsList.emplace_back(std::bind_front(fn, std::forward<Args>(pArgs)...));
        }

        void PushTask(std::function<void()>&& fn) {
            std::scoped_lock guard(m_mtx);
            m_jobsList.emplace_back(std::move(fn));
        }

        bool IsTaskWaiting() {
            std::scoped_lock guard(m_mtx);
            return !m_jobsList.empty();
        }

    private:
        void threadLoop(const std::stop_token& stopToken);
        void executeJob();
    };

}
