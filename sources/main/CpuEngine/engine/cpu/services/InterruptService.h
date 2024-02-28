//
// Created by pierr on 16/01/2024.
//
#pragma once

#include <list>

#include "EngineLib/services/IInterruptService.h"

namespace Astra::CPU::Core {

    class CpuEngine;

    class InterruptService : public IInterruptService
    {
    private:
        const std::list<const CpuEngine *>& m_interruptServiceList;

    public:
        explicit InterruptService(const std::list<const CpuEngine*>& inter) : m_interruptServiceList(inter) {}

        void SendDeviceInterrupt(int interruptId) override;
        void SendDeviceNonMaskableInterrupt(int interruptId) override;
        void SendDeviceRunningStatus(bool isRunning) override;
    };

}
