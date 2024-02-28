//
// Created by pierr on 30/07/2023.
//
#pragma once

#include <atomic>

namespace Astra::CPU {

    class IRunService
    {
    protected:
        std::atomic_bool m_running = false;

    public:
        virtual ~IRunService() = default;

        inline bool IsRunning() const {return m_running;}

        virtual void Run() = 0;
        virtual void Stop() = 0;
        virtual void Breakpoint() const = 0;

        virtual size_t GetSystemTicks() const = 0;
    };

}
