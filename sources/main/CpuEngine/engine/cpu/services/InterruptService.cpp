//
// Created by pierr on 16/01/2024.
//
#include "InterruptService.h"

#include "CpuEngine/engine/cpu/CpuEngine.h"

namespace Astra::CPU::Core {

    void InterruptService::SendDeviceRunningStatus(bool isRunning) {
        for (const auto& interruptService : m_interruptServiceList) {
            interruptService->GetRunService()->ForceRestartEngine(isRunning);
        }
    }

    void InterruptService::SendDeviceInterrupt(int interruptId) {
        for (const auto& interruptService : m_interruptServiceList) {
            interruptService->GetRunService()->Interrupt(false, interruptId);
        }
    }

    void InterruptService::SendDeviceNonMaskableInterrupt(int interruptId) {
        for (const auto& interruptService : m_interruptServiceList) {
            interruptService->GetRunService()->Interrupt(true, interruptId);
        }
    }
}
