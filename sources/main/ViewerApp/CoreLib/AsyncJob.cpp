//
// Created by pierr on 31/10/2023.
//
#include "AsyncJob.h"
#include "Commons/Profiling.h"

namespace Astra::UI::Core {
    AsyncJob::AsyncJob() {
        m_thread = std::jthread(&AsyncJob::threadLoop, this);
    }

    AsyncJob::~AsyncJob() {
        m_thread.request_stop();
        m_thread.join();
    }

    void AsyncJob::threadLoop(const std::stop_token& stopToken) {
        ENGINE_PROFILE_THREAD("AsyncJob");

        bool running = true;

        while (running) {
            if (stopToken.stop_requested()) {
                running = false;
            }

            executeJob();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void AsyncJob::executeJob() {
        ENGINE_PROFILE_FUNCTION();
        std::scoped_lock guard(m_mtx);

        for (const auto& job : m_jobsList) {
            job();
        }

        m_jobsList.clear();
    }
}
