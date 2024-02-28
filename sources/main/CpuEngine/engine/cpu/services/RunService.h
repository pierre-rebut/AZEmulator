//
// Created by pierr on 14/03/2023.
//
#pragma once

#include "CpuLog.h"
#include "EngineLib/services/IRunService.h"
#include "EngineLib/core/ICpuCore.h"

#include <thread>

namespace Astra::CPU::Core {

    class RunService : public IRunService
    {
    private:
        friend class RunExec;
        bool m_isRunnable = false;

        Ref<CpuLog> m_cpuLog;

        size_t m_elapsedCycle = 0;
        std::atomic_size_t m_totalCycle = 0;

        std::atomic_size_t m_cpuSpeed = 1;

        Scope<ICpuCore>& m_cpuCore;

        bool executeCycle() noexcept;

    public:
        RunService(size_t speed, Scope<ICpuCore>& cpuCore, const Ref<CpuLog>& cpuLog);

        inline size_t GetElapsedCycle() const {return m_elapsedCycle;}

        size_t GetSystemTicks() const override;

        void UpdateElapsedCycle();

        void Reset();

        inline size_t getCpuSpeed() const { return m_cpuSpeed; }

        void SetCpuSpeed(size_t pValue);

        void Run() override;
        void Stop() override;
        void Breakpoint() const override;

        void Interrupt(bool isNmi, int interruptId) noexcept;
        void ForceRestartEngine(bool isRunning);

        void SetRunnable(bool v) {m_isRunnable = v;}
    };

}
