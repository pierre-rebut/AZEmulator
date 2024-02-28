//
// Created by pierr on 30/10/2023.
//
#pragma once

#include <list>

#include "CpuEngine/engine/cpu/CpuEngine.h"

namespace Astra::CPU::Core {

    class RunExec
    {
    private:
        size_t m_systemClockCounter = 0;

    protected:
        bool m_isDebug = false;
        const std::atomic_bool& m_isRunning;
        const std::list<const CpuEngine*>& m_runEngines;

    public:
        RunExec(const std::atomic_bool& mIsRunning, const std::list<const CpuEngine*>& mRunEngines);
        virtual ~RunExec() = default;

        virtual bool Run();

        virtual bool ExecuteSystemClock();
        virtual bool ExecuteOneStep(const Ref<CpuEngine>& currentEngine);

        bool IsModeDebug() const {return m_isDebug;}

        void Reset();
        inline size_t GetSystemClock() const {return m_systemClockCounter;}

    protected:
        static bool isAllCoreReady();
        bool advanceUntilNextInstruction(const Ref<CpuEngine>& currentEngine, bool isModeStep);
    };

}
