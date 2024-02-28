//
// Created by pierr on 17/10/2023.
//
#pragma once

#include <thread>
#include <string>
#include <list>

#include "Commons/utils/Singleton.h"
#include "Commons/utils/UUID.h"
#include "CpuEngine/manager/EngineManager.h"
#include "RunExec.h"

namespace Astra::CPU::Core {

    class RunManager : public Singleton<RunManager>
    {
    public:
        static constexpr const char* NAME = "RunManager";

    private:
        Scope<RunExec> m_runner = nullptr;
        std::jthread m_thread;

        size_t m_previousSystemClockCounter = 0;
        int m_elapsedSystemClockCounter = 0;

        std::atomic_size_t m_systemClockSpeed = 500;
        std::atomic_bool m_isRunning = false;
        std::atomic_bool m_isThreadRunning = false;

        std::list<const CpuEngine*> m_runEngines{};

    public:
        RunManager();
        ~RunManager() override;

        void AddRunnableEngine(const CpuEngine* pEngine);
        void DelRunnableEngine(const CpuEngine* pEngine);

        inline size_t GetSystemClockCount() const {return m_runner->GetSystemClock();}
        inline int GetElapsedSystemClockCount() const {return m_elapsedSystemClockCounter;}

        inline void SetSystemClockSpeed(size_t newSpeed) {m_systemClockSpeed = newSpeed;}

        void UpdateElapsedCycle();

        inline bool isRunning() const { return m_isRunning; }
        bool Run(const Ref<CpuEngine>& currentEngine);
        void Stop();

        void Reset();
        void ClearAll();

        bool SetRunMode(bool isDebug);
        bool GetRunMode() const;

        bool ExecuteStep(const Ref<CpuEngine>& currentEngine, int nbStep);

        const RunExec* GetRunner() const {return m_runner.get();}

        void UpdateOrderPriorityList();

    private:
        void threadLoop(const std::stop_token& stopToken);
        void runManagerSleep();
    };

}
