//
// Created by pierr on 30/10/2023.
//
#include "RunExec.h"

#include "CpuEngine/manager/EngineManager.h"

#include <algorithm>

namespace Astra::CPU::Core {

    RunExec::RunExec(const std::atomic_bool& mIsRunning, const std::list<const CpuEngine*>& mRunEngines) :
            m_isRunning(mIsRunning), m_runEngines(mRunEngines) {}

    bool RunExec::Run() {
        LOG_CPU_DEBUG("[RunExec] Run");

        if (!isAllCoreReady()) {
            LOG_CPU_DEBUG("[RunExec] Run failed");
            return false;
        }

        LOG_CPU_DEBUG("[RunExec] Run END");
        return true;
    }

    bool RunExec::ExecuteSystemClock() {
        bool isOk = true;

        for (const auto& engine: m_runEngines) {
            const auto& runService = engine->GetRunService();
            if (runService->IsRunning() && (m_systemClockCounter % runService->getCpuSpeed()) == 0) {
                isOk &= runService->executeCycle();
            }
        }

        m_systemClockCounter++;
        return isOk;
    }

    bool RunExec::isAllCoreReady() {
        LOG_CPU_TRACE("[RunExec] try isAllCoreReady");

        return std::ranges::all_of(EngineManager::Get().getEngines(), [](const auto& engine) {
            if (!engine.second->IsCoreInit()) {
                NOTIFY_WARN("Invalid engine {} (UUID: {}), run failed", engine.second->GetName(), engine.first);
                return false;
            }
            return true;
        });
    }

    bool RunExec::ExecuteOneStep(const Ref<CpuEngine>& currentEngine) {
        LOG_CPU_DEBUG("[RunExec] ExecuteStep");

        if (!isAllCoreReady()) {
            return false;
        }

        if (!advanceUntilNextInstruction(currentEngine, true)) {
            return false;
        }

        bool ret = RunExec::ExecuteSystemClock();

        LOG_CPU_DEBUG("[RunExec] ExecuteStep END");
        return ret;
    }

    bool RunExec::advanceUntilNextInstruction(const Ref<CpuEngine>& currentEngine, bool isModeStep) {
        for (int i = 0; i < 100000; i++) {
            if (currentEngine->GetCore()->IsComplete() || (!isModeStep && !m_isRunning)) {
                return true;
            }

            RunExec::ExecuteSystemClock();
        }

        return false;
    }

    void RunExec::Reset() {
        LOG_CPU_DEBUG("[RunExec] Reset");
        m_systemClockCounter = 0;
        LOG_CPU_DEBUG("[RunExec] Reset END");
    }
}
