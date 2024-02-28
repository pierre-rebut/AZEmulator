//
// Created by pierr on 11/03/2023.
//

#include "CpuEngine.h"

#include "Commons/Log.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "CpuEngine/manager/running/RunManager.h"

namespace Astra::CPU::Core {

    CpuEngine::CpuEngine(const CpuCreateData& pData) : Device(pData.uuid, DeviceType::ENGINE, m_name),
                                                       m_name(pData.name), m_autostart(pData.autostart), m_orderPriority(pData.orderPriority),
                                                       m_cpuCoreName{pData.coreLibDir, pData.coreLibName} {
        LOG_CPU_INFO("[CpuEngine] Init");

        m_log = CreateRef<CpuLog>(deviceUUID, m_name);

        m_runService = CreateRef<RunService>(pData.speed, m_cpuCore, m_log);
        m_entitiesService = CreateScope<EntitiesService>();
        m_interruptService = CreateScope<InterruptService>(m_interruptServiceList);

        for (const auto& param: pData.hardParameters) {
            m_hardParameters.emplace_back(param);
        }

        LOG_CPU_INFO("[CpuEngine] Init END");
    }

    bool CpuEngine::LoadCore(const IFactory* coreFactory) noexcept {
        LOG_CPU_INFO("[CpuEngine] LoadCore for engine {}", deviceUUID);

        try {
            m_cpuCoreFactory = coreFactory;
            m_coreConfigInfo = m_cpuCoreFactory->GetCoreConfig();

            m_cpuCore = m_cpuCoreFactory->CreateNewCore(m_entitiesService.get());
            m_cpuCore->Init(m_runService.get(), m_interruptService.get());

            m_hardParameters.resize(m_coreConfigInfo->hardParameters.size());

            m_runService->SetRunnable(m_cpuCoreFactory->isRunnable);
            if (m_cpuCoreFactory->isRunnable) {
                RunManager::Get().AddRunnableEngine(this);
            }

            m_log->success("Lib {} loaded", m_cpuCoreName.second);
        } catch (const std::exception& e) {
            m_log->error("Lib {} error : {}", m_cpuCoreName.second, e.what());
            return false;
        }

        LOG_CPU_INFO("[CpuEngine] LoadCore END");
        return true;
    }

    void CpuEngine::UnloadCore() {
        LOG_CPU_INFO("[CpuEngine] UnloadCore for engine {}", deviceUUID);

        m_entitiesService->Reset();

        if (m_cpuCoreFactory->isRunnable) {
            RunManager::Get().DelRunnableEngine(this);
        }

        m_cpuCore = nullptr;
        m_cpuCoreFactory = nullptr;
        m_coreConfigInfo = nullptr;

        LOG_CPU_INFO("[CpuEngine] UnloadCore END");
    }

    bool CpuEngine::UpdateHardParameters() noexcept {
        LOG_CPU_INFO("[CpuEngine] UpdateHardParameters");

        try {
            return m_cpuCore->UpdateHardParameters(m_hardParameters);
        } catch (const std::exception& e) {
            LOG_CPU_ERROR("EngineDevice error UpdateHardParameters : {}", e.what());
            return false;
        }

        LOG_CPU_INFO("[CpuEngine] UpdateHardParameters END");
    }

    LARGE CpuEngine::Fetch(DataFormat fmt, size_t address) {
        return m_cpuCore->Fetch(fmt, address);
    }

    void CpuEngine::Push(DataFormat fmt, size_t address, LARGE value) {
        m_cpuCore->Push(fmt, address, value);
    }

    const std::vector<std::pair<size_t, size_t>>& CpuEngine::GetDeviceAddressList() const {
        static const std::vector<std::pair<size_t, size_t>> defaultList = {};
        return m_cpuCore ? *m_cpuCore->GetDeviceAddressList() : defaultList;
    }

    int CpuEngine::FetchReadOnly(size_t address) const {
        return m_cpuCore->FetchReadOnly(address);
    }

    bool CpuEngine::ConnectCpuInterrupt(const Ref<CpuEngine>& otherEngine) {
        LOG_CPU_DEBUG("[Device] Connect cpu {} to device {}", otherEngine->GetName(), GetName());

        const auto otherEnginePtr = otherEngine.get();

        const auto exists = std::ranges::find_if(m_interruptServiceList, [&otherEnginePtr](const auto& interruptService){
            return interruptService == otherEnginePtr;
        });

        bool ret = (exists == m_interruptServiceList.end());
        if (ret) {
            const auto pos = std::ranges::find_if(m_interruptServiceList, [&otherEnginePtr](const auto& interruptService){
                return otherEnginePtr->GetOrderPriority() < interruptService->GetOrderPriority();
            });

            m_interruptServiceList.insert(pos, otherEnginePtr);
        }

        LOG_CPU_DEBUG("[Device] Connect cpu END");
        return ret;
    }

    void CpuEngine::DisconnectCpuInterrupt(const CpuEngine* connectedEngine) {
        LOG_CPU_DEBUG("[Device] Disconnect cpu {} to device {}", connectedEngine->GetName(), GetName());

        std::erase_if(m_interruptServiceList, [&connectedEngine](const auto& interruptService){
            return interruptService == connectedEngine;
        });

        LOG_CPU_DEBUG("[Device] Disconnect cpu END");
    }

    void CpuEngine::UpdateInterruptListOrder() {
        LOG_CPU_DEBUG("[Device] UpdateInterruptListOrder");

        m_interruptServiceList.sort([](const auto& a, const auto& b){
            return a->GetOrderPriority() < b->GetOrderPriority();
        });

        LOG_CPU_DEBUG("[Device] UpdateInterruptListOrder END");
    }
}
