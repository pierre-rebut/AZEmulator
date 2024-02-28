//
// Created by pierr on 27/08/2023.
//

#pragma once

#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "CpuEngine/manager/buses/DataBusManager.h"
#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "CpuEngine/manager/LogManager.h"
#include "CpuEngine/manager/running/RunManager.h"

namespace Astra::CPU::Core {

    class CpuContext
    {
    private:
        LogManager m_cpuLog;
        EngineManager m_engines;
        DevicesManager m_devices;
        DataBusManager m_dataBuses;
        CoreLibManager m_coreLib;
        RunManager m_run;

    public:
        CpuContext() {
            LogManager::SetSingleton(&m_cpuLog);
            EngineManager::SetSingleton(&m_engines);
            DevicesManager::SetSingleton(&m_devices);
            DataBusManager::SetSingleton(&m_dataBuses);
            CoreLibManager::SetSingleton(&m_coreLib);
            RunManager::SetSingleton(&m_run);
        }

        ~CpuContext() {
            RunManager::SetSingleton(nullptr);
            CoreLibManager::SetSingleton(nullptr);
            DataBusManager::SetSingleton(nullptr);
            DevicesManager::SetSingleton(nullptr);
            EngineManager::SetSingleton(nullptr);
            LogManager::SetSingleton(nullptr);
        }
    };

} // Astra
