//
// Created by pierr on 14/03/2023.
//

#include "RunService.h"

#include "CpuEngine/manager/running/RunManager.h"

namespace Astra::CPU::Core {
    RunService::RunService(size_t speed, Scope<ICpuCore>& cpuCore, const Ref<CpuLog>& cpuLog)
            : m_cpuLog(cpuLog), m_cpuSpeed(speed), m_cpuCore(cpuCore) {
    }

    void RunService::Run() {
        if (!m_cpuCore) {
            m_cpuLog->error("Invalid core, skipped");
            return;
        }

        m_running = true;
    }

    void RunService::Stop() {
        m_running = false;
    }

    bool RunService::executeCycle() noexcept {
        try {
            m_cpuCore->Execute();
            m_totalCycle++;
        } catch (const std::exception& e) {
            LOG_CPU_ERROR("[RunService] an error occurred during execution: {}", e.what());
            m_cpuLog->error("Instruction failed: {}", e.what());
            m_totalCycle++;
            return false;
        }

        return true;
    }

    void RunService::SetCpuSpeed(size_t pValue) {
        m_cpuSpeed = pValue;

        LOG_CPU_INFO("RunService: setting cpu speed at {}", pValue);
        m_cpuLog->warn("updating cpu speed {}", pValue);
    }

    void RunService::Reset() {
        m_running = false;
        m_totalCycle = 0;
    }

    void RunService::UpdateElapsedCycle() {
        m_elapsedCycle = m_totalCycle;
        m_totalCycle = 0;
    }

    void RunService::Breakpoint() const {
        RunManager::Get().Stop();
    }

    size_t RunService::GetSystemTicks() const {
        return RunManager::Get().GetSystemClockCount();
    }

    void RunService::Interrupt(bool isNmi, int interruptId) noexcept {
        try {
            m_cpuCore->Interrupt(isNmi, interruptId);
            m_running = m_isRunnable;
        } catch (const std::exception& e) {
            LOG_CPU_ERROR("[RunService] an error occurred during interruption: {}", e.what());
            m_cpuLog->error("Interrupt failed: {}", e.what());
        }
    }

    void RunService::ForceRestartEngine(bool isRunning) {
        m_running = isRunning;
    }
}
