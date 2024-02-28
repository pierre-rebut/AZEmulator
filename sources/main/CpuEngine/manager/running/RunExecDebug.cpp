//
// Created by pierr on 30/10/2023.
//
#include "RunExecDebug.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {

    RunExecDebug::RunExecDebug(const std::atomic_bool& mIsRunning, const std::list<const CpuEngine*>& mRunEngines) : RunExec(mIsRunning, mRunEngines) {
        m_isDebug = true;
    }

    bool RunExecDebug::Run() {
        LOG_CPU_DEBUG("[RunExecDebug] RUN");

        if (!m_currentEngine || !m_currentEngine->GetRunService()->IsRunning()) {
            return false;
        }

        std::scoped_lock guard(m_debugMtx);
        m_debugExec.clear();

        LOG_CPU_DEBUG("[RunExecDebug] Run super");
        return RunExec::Run();
    }

    bool RunExecDebug::ExecuteSystemClock() {
        bool isOk = advanceUntilNextInstruction(m_currentEngine, false);
        if (!isOk) {
            return false;
        }

        RunExec::ExecuteSystemClock();

        std::scoped_lock guard(m_debugMtx);

        m_debugExec.emplace_front(m_currentEngine->GetCore()->DebugExecute());
        m_instCount++;

        if (m_debugExec.size() > MAX_NB_LOG) {
            m_debugExec.pop_back();
        }

        return true;
    }

    bool RunExecDebug::ExecuteOneStep(const Ref<CpuEngine>& currentEngine) {
        LOG_CPU_DEBUG("[RunExecDebug] ExecuteStep");

        bool ret = RunExec::ExecuteOneStep(currentEngine);

        if (ret) {
            std::scoped_lock guard(m_debugMtx);

            m_debugExec.emplace_front(currentEngine->GetCore()->DebugExecute());
            m_instCount++;

            if (m_debugExec.size() > MAX_NB_LOG) {
                m_debugExec.pop_back();
            }
        }

        LOG_CPU_DEBUG("[RunExecDebug] ExecuteStep END");
        return ret;
    }
}
