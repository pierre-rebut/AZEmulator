//
// Created by pierr on 25/08/2023.
//

#pragma once

#include <unordered_map>

#include "EngineLib/data/Base.h"
#include "Commons/utils/UUID.h"
#include "Commons/utils/Singleton.h"

#include "DataBus.h"
#include "CpuEngine/data/BusCreateData.h"

namespace Astra::CPU::Core {
    class DataBusManager : public Singleton<DataBusManager>
    {
    public:
        static constexpr const char* NAME = "DataBusManager";

    private:
        std::unordered_map<UUID, Ref<DataBus>> m_dataBuses;

    public:
        void Init(const std::vector<BusCreateData>& busList);

        void RemoveAllBus() {
            m_dataBuses.clear();
        }

        void CreateNewBus(const std::string& name, size_t busSize);

        void RemoveDataBus(const Ref<DataBus>& dataBus);

        void RemoveDeviceLink(const Ref<Device>& device) const;

        inline const std::unordered_map<UUID, Ref<DataBus>>& GetDataBuses() const {return m_dataBuses;}

        Ref<DataBus> GetDataBusByUUID(UUID busUUID) const {
            if (!m_dataBuses.contains(busUUID)) {
                return nullptr;
            }

            return m_dataBuses.at(busUUID);
        }

        bool RefreshDeviceConnections(const Ref<Device>& device) const;
    };
}
