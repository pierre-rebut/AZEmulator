//
// Created by pierr on 25/08/2023.
//

#include "DataBusManager.h"

#include "Commons/Log.h"
#include "CpuEngine/manager/EngineManager.h"

namespace Astra::CPU::Core {

    void DataBusManager::Init(const std::vector<BusCreateData>& busList) {
        LOG_CPU_DEBUG("[DataBusManager] Init");

        for (const auto& busInfo: busList) {
            m_dataBuses[busInfo.uuid] = CreateRef<DataBus>(busInfo);
        }

        LOG_CPU_DEBUG("[DataBusManager] Init END");
    }

    void DataBusManager::CreateNewBus(const std::string& name, size_t busSize) {
        LOG_CPU_DEBUG("[DataBusManager] CreateNewBus {}", name);

        auto busUUID = UUIDGen::New();
        m_dataBuses[busUUID] = CreateRef<DataBus>(BusCreateData{busUUID, name, busSize});

        LOG_CPU_DEBUG("[DataBusManager] CreateNewBus END");
    }

    void DataBusManager::RemoveDataBus(const Ref<DataBus>& dataBus) {
        LOG_CPU_DEBUG("[DataBusManager] RemoveDataBus {}", dataBus->deviceUUID);

        EngineManager::Get().UnlinkDatabus(dataBus);
        m_dataBuses.erase(dataBus->deviceUUID);

        LOG_CPU_DEBUG("[DataBusManager] RemoveDataBus END");
    }

    void DataBusManager::RemoveDeviceLink(const Ref<Device>& device) const {
        LOG_CPU_DEBUG("[DataBusManager] RemoveDeviceLink {}", device->GetName());

        for (const auto& [busUUID, bus] : m_dataBuses) {
            bus->DisconnectDevice(device);
        }

        LOG_CPU_DEBUG("[DataBusManager] RemoveDeviceLink END");
    }

    bool DataBusManager::RefreshDeviceConnections(const Ref<Device>& device) const {
        LOG_CPU_DEBUG("[DataBusManager] RemoveDeviceLink {} (UUID: {})", device->GetName(), device->deviceUUID);

        bool isError = false;

        for (const auto& [busUUID, bus] : m_dataBuses) {
            isError |= bus->RefreshDeviceConnections(device);
        }

        LOG_CPU_DEBUG("[DataBusManager] RemoveDeviceLink END");
        return isError;
    }
}