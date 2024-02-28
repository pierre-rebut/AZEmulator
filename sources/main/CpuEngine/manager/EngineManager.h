//
// Created by pierr on 19/07/2023.
//

#pragma once

#include <map>

#include "Commons/utils/UUID.h"
#include "Commons/utils/Singleton.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "CpuEngine/data/CpuCreateData.h"
#include "CpuEngine/manager/buses/DataBus.h"

namespace Astra::CPU::Core {

    using EngineList = std::unordered_map<UUID, Ref<CpuEngine>>;

    class EngineManager : public Singleton<EngineManager>
    {
    public:
        static constexpr const char* NAME = "EngineManager";

    private:
        bool m_isInit = false;
        EngineList m_engines{};

    public:
        inline bool isInit() const { return m_isInit; }

        void InitEngines(const std::vector<CpuCreateData>& pCpuInfos);
        void ClearEngines();
        void Reset() const;

        UUID AddEngine(const CpuCreateData& data);
        void RemoveEngine(UUID engineUUID);
        Ref<CpuEngine> GetEngineByUUID(UUID pUuid) const;

        const EngineList& getEngines() const { return m_engines; }

        void UnbindDevice(const Ref<IDevice>& device);

        void UpdateOrderPriorityList() const;

        void UnlinkDatabus(const Ref<DataBus>& databus);

    private:
        Ref <CpuEngine> createNewEngine(const CpuCreateData& data) const;
    };

}
