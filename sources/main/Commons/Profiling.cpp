//
// Created by pierr on 14/09/2021.
//

#ifdef PROFILING_ENABLE

#include "Profiling.h"

#include <utility>

namespace Astra {
    ProfilingTimer::ProfilingTimer(const char* name)
            : m_Name(name) {
        mStartTimepoint = std::chrono::steady_clock::now();
    }

    ProfilingTimer::~ProfilingTimer() {
        if (!m_Stopped) {
            Stop();
        }
    }

    void ProfilingTimer::Stop() {
        auto endTimePoint = std::chrono::steady_clock::now();
        auto highResStart = FloatingPointMicroseconds{mStartTimepoint.time_since_epoch()};
        auto elapsedTime =
                std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch() -
                std::chrono::time_point_cast<std::chrono::microseconds>(mStartTimepoint).time_since_epoch();

        Profiling::Get().WriteProfile({m_Name, highResStart, elapsedTime, std::this_thread::get_id()});

        m_Stopped = true;
    }


    ProfilingSession::ProfilingSession(std::string pName)
            : Name(std::move(pName)) {}
}

#endif