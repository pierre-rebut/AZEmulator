//
// Created by pierr on 19/07/2023.
//

#include "EngineManager.h"

#include "Commons/Log.h"
#include "CpuEngine/exception/EngineException.h"

#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "CpuEngine/manager/running/RunManager.h"

namespace Astra::CPU::Core {

    void EngineManager::InitEngines(const std::vector<CpuCreateData>& pCpuInfos) {
        LOG_CPU_DEBUG("[CpuEngine] InitEngines");

        const auto& libManager = CoreLibManager::Get();

        for (const auto& cpuInfo: pCpuInfos) {
            auto engine = CreateRef<CpuEngine>(cpuInfo);

            DevicesManager::Get().addEngineDevice(engine);

            libManager.LoadEngineCoreLib(engine);

            m_engines[cpuInfo.uuid] = engine;
        }

        m_isInit = true;
        LOG_CPU_DEBUG("[CpuEngine] InitEngines END");
    }

    void EngineManager::ClearEngines() {
        LOG_CPU_DEBUG("[CpuEngine] ClearEngines");

        m_engines.clear();

        m_isInit = false;
        LOG_CPU_DEBUG("[CpuEngine] ClearEngines END");
    }

    UUID EngineManager::AddEngine(const CpuCreateData& engineData) {
        LOG_CPU_DEBUG("[CpuEngine] AddEngine {}", engineData.uuid);

        auto engine = CreateRef<CpuEngine>(engineData);
        m_engines[engineData.uuid] = engine;

        DevicesManager::Get().addEngineDevice(engine);

        NOTIFY_SUCCESS("Engine {} ({}) added", engineData.name, engineData.uuid);
        LOG_CPU_DEBUG("[CpuEngine] AddEngine END");
        return engineData.uuid;
    }

    void EngineManager::RemoveEngine(UUID engineUUID) {
        LOG_CPU_DEBUG("[CpuEngine] RemoveEngine {}", engineUUID);
        EngineException::assertV(m_engines.contains(engineUUID), "[CpuEngine] Invalid cpu uuid {}", engineUUID);

        const auto& engine = m_engines.at(engineUUID);
        CoreLibManager::Get().UnloadEngineCoreLib(engine);

        DevicesManager::Get().RemoveDevice(engineUUID);

        m_engines.erase(engineUUID);

        NOTIFY_SUCCESS("Engine {} deleted", engineUUID);
        LOG_CPU_DEBUG("[CpuEngine] RemoveEngine END");
    }

    Ref<CpuEngine> EngineManager::GetEngineByUUID(UUID pUuid) const {
        if (!m_engines.contains(pUuid)) {
            return nullptr;
        }

        return m_engines.at(pUuid);
    }

    void EngineManager::Reset() const {
        LOG_CPU_DEBUG("[CpuEngine] Reset");

        std::vector<Ref<CpuEngine>> tmpResetList;
        for (const auto& [engineUUID, engine] : m_engines) {
            tmpResetList.emplace_back(engine);
        }

        std::ranges::sort(tmpResetList.begin(), tmpResetList.end(), [](const auto& a, const auto& b){
            return a->GetOrderPriority() < b->GetOrderPriority();
        });

        for (const auto& engine : tmpResetList) {
            engine->Reset();
        }

        RunManager::Get().Reset();

        LOG_CPU_DEBUG("[CpuEngine] Reset END");
    }

    void EngineManager::UnbindDevice(const Ref<IDevice>& device) {
        LOG_CPU_DEBUG("[CpuEngine] UnbindDevice");

        for (const auto& [engineUUID, engine] : m_engines) {
            for (const auto& [bindName, binding] : engine->GetEntities().GetDevices()) {
                if (binding->GetValue() == device) {
                    binding->SetValue(nullptr);
                }
            }
        }

        LOG_CPU_DEBUG("[CpuEngine] UnbindDevice END");
    }

    void EngineManager::UpdateOrderPriorityList() const {
        LOG_CPU_DEBUG("[CpuEngine] UpdateOrderPriorityList");

        for (const auto& [engineUUID, engine] : m_engines) {
            engine->UpdateInterruptListOrder();
        }

        LOG_CPU_DEBUG("[CpuEngine] UpdateOrderPriorityList END");
    }

    void EngineManager::UnlinkDatabus(const Ref<DataBus>& databus) {
        LOG_CPU_DEBUG("[EngineManager] UnlinkDatabus {}", databus->deviceUUID);

        for (const auto& [engineUUID, engine] : m_engines) {
            engine->GetEntities().UnlinkDatabus(databus);
        }

        LOG_CPU_DEBUG("[EngineManager] UnlinkDatabus END");
    }
}