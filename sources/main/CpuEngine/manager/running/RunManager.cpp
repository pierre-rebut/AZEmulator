//
// Created by pierr on 17/10/2023.
//
#include "RunManager.h"

#include "Commons/Log.h"
#include "CpuEngine/exception/EngineException.h"
#include "Commons/AstraMessage.h"
#include "Commons/Profiling.h"
#include "RunExecDebug.h"

namespace Astra::CPU::Core {
    RunManager::RunManager() {
        LOG_CPU_DEBUG("[RunManager] Init");

        m_runner = CreateScope<RunExec>(m_isRunning, m_runEngines);
        m_thread = std::jthread(&RunManager::threadLoop, this);

        LOG_CPU_DEBUG("[RunManager] Init END");
    }

    RunManager::~RunManager() {
        LOG_CPU_DEBUG("[RunManager] Destroy");

        m_thread.request_stop();

        LOG_CPU_DEBUG("[RunManager] Destroy END");
    }

    void RunManager::Reset() {
        LOG_CPU_DEBUG("[RunManager] Reset CPU");
        EngineException::assertV(EngineManager::Get().isInit(), "[RunManager] CPUs not Init");

        m_isRunning = false;
        m_isThreadRunning = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        LOG_CPU_TRACE("[RunManager] Reset all core");
        for (const auto& engine: m_runEngines) {
            auto runService = engine->GetRunService();
            if (engine->IsAutoStart()) {
                runService->Run();
            } else {
                runService->Stop();
            }
        }

        m_runner->Reset();

        LOG_CPU_DEBUG("[RunManager] Reset CPU END");
    }

    void RunManager::threadLoop(const std::stop_token& stopToken) {
        ENGINE_PROFILE_THREAD("CpuCoreLoop");
        LOG_CPU_DEBUG("RunService: creating thread");

        while (!stopToken.stop_requested()) {
            if (m_isThreadRunning) {
                bool isOK = m_runner->ExecuteSystemClock();
                if (!isOK) {
                    Stop();
                    NOTIFY_WARN("Error during execution");
                }

                runManagerSleep();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        LOG_CPU_DEBUG("RunService: destroy thread");
    }

    bool RunManager::Run(const Ref<CpuEngine>& currentEngine) {
        LOG_CPU_DEBUG("[RunManager] Run");
        EngineException::assertV(EngineManager::Get().isInit(), "[RunManager] EngineManager not Init");

        if (m_isRunning) {
            return false;
        }

        if (m_runner->IsModeDebug()) {
            auto debugRunner = (RunExecDebug*) m_runner.get();
            debugRunner->SetCurrentEngine(currentEngine);
        }

        if (!m_runner->Run()) {
            return false;
        }

        m_isRunning = true;
        m_isThreadRunning = true;

        NOTIFY_INFO("Engines started");
        LOG_CPU_DEBUG("[RunManager] Run");
        return true;
    }

    void RunManager::Stop() {
        if (!m_isRunning) {
            return;
        }

        LOG_CPU_DEBUG("[RunManager] Stop");

        m_isRunning = false;
        m_isThreadRunning = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        NOTIFY_INFO("Engines stopped");
        LOG_CPU_DEBUG("[RunManager] Stop END");
    }

    bool RunManager::SetRunMode(bool isDebug) {
        if (m_isRunning) {
            return false;
        }

        if (isDebug) {
            m_runner = CreateScope<RunExecDebug>(m_isRunning, m_runEngines);
        } else {
            m_runner = CreateScope<RunExec>(m_isRunning, m_runEngines);
        }

        return true;
    }

    bool RunManager::GetRunMode() const {
        return m_runner->IsModeDebug();
    }

    bool RunManager::ExecuteStep(const Ref<CpuEngine>& currentEngine, int nbStep) {
        LOG_CPU_DEBUG("[RunManager] ExecuteStep {}", nbStep);

        if (m_isRunning || !currentEngine->GetRunService()->IsRunning()) {
            return false;
        }

        m_isRunning = true; // todo fix pause lors mode step (bug)

        for (int i = 0; i < nbStep; i++) {
            m_runner->ExecuteOneStep(currentEngine);
        }

        m_isRunning = false;

        LOG_CPU_DEBUG("[RunManager] ExecuteStep END");
        return true;
    }

    void RunManager::runManagerSleep() {
        if (m_systemClockSpeed >= 1000) {
            std::this_thread::sleep_for(std::chrono::microseconds(m_systemClockSpeed));
        } else {
            for (size_t i = 0; i < m_systemClockSpeed; i++);

            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    void RunManager::UpdateElapsedCycle() {
        auto systemClockCounter = m_runner->GetSystemClock();
        m_elapsedSystemClockCounter = static_cast<int>(systemClockCounter - m_previousSystemClockCounter);
        m_previousSystemClockCounter = systemClockCounter;
    }

    void RunManager::AddRunnableEngine(const CpuEngine* pEngine) {
        LOG_CPU_DEBUG("[RunManager] AddRunnableEngine {}", pEngine->deviceUUID);

        auto pos = std::ranges::find_if(m_runEngines.begin(), m_runEngines.end(), [pEngine](const CpuEngine *other){
            return pEngine->GetOrderPriority() < other->GetOrderPriority();
        });

        m_runEngines.insert(pos, pEngine);

        LOG_CPU_DEBUG("[RunManager] AddRunnableEngine END");
    }

    void RunManager::DelRunnableEngine(const CpuEngine* pEngine) {
        LOG_CPU_DEBUG("[RunManager] DelRunnableEngine {}", pEngine->deviceUUID);

        std::erase_if(m_runEngines, [pEngine](const CpuEngine* engine){
            return pEngine == engine;
        });

        LOG_CPU_DEBUG("[RunManager] DelRunnableEngine END");
    }

    void RunManager::ClearAll() {
        LOG_CPU_DEBUG("[RunManager] ClearAll");

        Stop();

        m_runEngines.clear();
        m_runner = CreateScope<RunExec>(m_isRunning, m_runEngines);
        m_previousSystemClockCounter = 0;
        m_elapsedSystemClockCounter = 0;

        LOG_CPU_DEBUG("[RunManager] ClearAll END");
    }

    void RunManager::UpdateOrderPriorityList() {
        LOG_CPU_DEBUG("[RunManager] UpdateOrderPriorityList");

        m_runEngines.sort([](const auto& a, const auto& b){
            return a->GetOrderPriority() < b->GetOrderPriority();
        });

        LOG_CPU_DEBUG("[RunManager] UpdateOrderPriorityList");
    }
}
