//
// Created by pierr on 11/03/2023.
//
#pragma once

#include <list>

#include "EngineLib/data/Base.h"

#include "EngineLib/IFactory.h"
#include "EngineLib/core/ICpuCore.h"

#include "CpuEngine/engine/cpu/services/RunService.h"
#include "CpuEngine/engine/cpu/services/EntitiesService.h"
#include "CpuEngine/data/CpuCreateData.h"

#include "CpuEngine/engine/Device.h"
#include "CpuEngine/engine/cpu/services/InterruptService.h"

namespace Astra::CPU::Core {

    class CpuEngine : public Device
    {
    private:
        std::string m_name;
        bool m_autostart;
        int m_orderPriority = 1;

        Ref<CpuLog> m_log;

        Ref<RunService> m_runService;
        Scope<EntitiesService> m_entitiesService;
        Scope<InterruptService> m_interruptService;

        Scope<ICpuCore> m_cpuCore;
        const IFactory* m_cpuCoreFactory = nullptr;
        std::pair<std::string, std::string> m_cpuCoreName;

        const CoreConfigInfo* m_coreConfigInfo = nullptr;
        std::vector<int> m_hardParameters{};

        std::list<const CpuEngine*> m_interruptServiceList{};

    public:
        explicit CpuEngine(const CpuCreateData& pData);

        virtual bool LoadCore(const IFactory* coreFactory) noexcept;
        virtual void UnloadCore();
        ICpuCore* GetCore() const {return m_cpuCore.get();}

        inline const CoreConfigInfo* GetCoreConfigInfo() const {return m_coreConfigInfo;}

        inline const std::pair<std::string, std::string>& GetCpuCoreName() const {return m_cpuCoreName;}
        void SetCpuCoreName(const std::string& cpuCoreDir, const std::string& cpuCoreName) {m_cpuCoreName = {cpuCoreDir, cpuCoreName};}

        inline bool IsCoreValid() const {return m_cpuCore.operator bool();}
        inline bool IsCoreInit() const {return IsCoreValid() && m_cpuCore->IsInit();}
        inline bool IsRunnable() const {return m_cpuCoreFactory && m_cpuCoreFactory->isRunnable;}
        inline bool IsAutoStart() const { return m_autostart; }
        inline void SetAutoStart(bool value) { m_autostart = value; }

        inline int GetOrderPriority() const {return m_orderPriority;}
        inline void SetOrderPriority(int priority) { m_orderPriority = priority;}

        inline void Reset() const {m_cpuCore->Reset();}
        inline const EntitiesService& GetEntities() const {return *m_entitiesService;}
        inline const Ref<RunService>& GetRunService() const { return m_runService; }

        const std::string& GetName() const override{ return m_name; }
        inline void SetName(const std::string_view& mName) { m_name = mName; }

        std::vector<int>& GetHardParameters() {return m_hardParameters;}
        bool UpdateHardParameters() noexcept;

        // device
        LARGE Fetch(DataFormat fmt, size_t address) override;
        void Push(DataFormat fmt, size_t address, LARGE value) override;
        int FetchReadOnly(size_t address) const override;

        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override;

        const auto& GetInterruptServices() const {return m_interruptServiceList;}
        bool ConnectCpuInterrupt(const Ref<CpuEngine>& pInterruptService);
        void DisconnectCpuInterrupt(const CpuEngine* connectedEngine);

        void UpdateInterruptListOrder();
    };

}
