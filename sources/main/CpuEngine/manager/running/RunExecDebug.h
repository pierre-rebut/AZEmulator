//
// Created by pierr on 30/10/2023.
//
#pragma once

#include "RunExec.h"

#include <string>
#include <list>

namespace Astra::CPU::Core {

    class RunExecDebug : public RunExec
    {
    public:
        static constexpr const int MAX_NB_LOG = 100;

    private:
        Ref<CpuEngine> m_currentEngine = nullptr;
        std::list<std::vector<int>> m_debugExec{};
        size_t m_instCount = 0;

        std::mutex m_debugMtx;

    public:
        RunExecDebug(const std::atomic_bool& mIsRunning, const std::list<const CpuEngine*>& mRunEngines);

        bool Run() override;

        bool ExecuteSystemClock() override;
        bool ExecuteOneStep(const Ref<CpuEngine>& currentEngine) override;

        std::mutex& GetDebugMutex() {return m_debugMtx;}
        const std::list<std::vector<int>>& GetDebugLog() const {return m_debugExec;}

        void SetCurrentEngine(const Ref<CpuEngine>& currentEngine) {m_currentEngine = currentEngine;}

        size_t GetInstructionCount() const {return m_instCount;}
    };

}
