//
// Created by pierr on 24/08/2023.
//

#pragma once

#include <vector>

#include "AstraRunException.h"

#include "EngineLib/services/IRunService.h"
#include "EngineLib/services/IInterruptService.h"
#include "EngineLib/data/Types.h"

namespace Astra::CPU {

    class ICpuCore
    {
    protected:
        IRunService* m_runService = nullptr;
        IInterruptService* m_deviceService = nullptr;

    public:
        virtual ~ICpuCore() = default;

        void Init(IRunService* runService, IInterruptService* deviceService) {
            m_runService = runService;
            m_deviceService = deviceService;
        }

        virtual bool IsInit() const = 0;
        virtual bool IsComplete() const { return true; }

        virtual void Reset() { /* do nothing */ }
        virtual void Execute() { throw AstraRunException("Not implemented"); }
        virtual void Interrupt(bool isNmi, int interruptId) { /* do nothing */ };

        virtual std::vector<int> DebugExecute() const { return {}; }

        virtual bool LoadFromStream(std::basic_istream<char>& stream) { return false; }

        virtual bool UpdateHardParameters(const std::vector<int>& hardParameters) { return true; }

        virtual const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const { return nullptr; }

        virtual LARGE Fetch(DataFormat fmt, size_t address) { return 0; }
        virtual void Push(DataFormat fmt, size_t address, LARGE value) { /* do nothing */ }
        virtual int FetchReadOnly(size_t address) const { return -1; }
    };

} // Astra
