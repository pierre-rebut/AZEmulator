//
// Created by pierr on 15/10/2023.
//
#pragma once

namespace Astra::CPU {

    class IInterruptService
    {
    public:
        virtual ~IInterruptService() = default;

        virtual void SendDeviceInterrupt(int interruptId) = 0;
        virtual void SendDeviceNonMaskableInterrupt(int interruptId) = 0;
        virtual void SendDeviceRunningStatus(bool isRunning = true) = 0;
    };

}
