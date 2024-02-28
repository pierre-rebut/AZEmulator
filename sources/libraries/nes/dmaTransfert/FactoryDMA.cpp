//
// Created by pierr on 16/10/2023.
//
#include "FactoryDMA.h"
#include "CoreDMA.h"

namespace Astra::CPU::Lib {
    FactoryDMA::FactoryDMA() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> FactoryDMA::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreDMA>();
#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(dmaData);
        REGISTER(dmaAddr);
        REGISTER(dmaPage);
        REGISTER(oamAddr);

#define FLAG(name, index) entitiesService->AddNewFlags(#name, &core->flags.reg, sizeof(core->flags.reg), index);

        FLAG(isTransfer, 0);
        FLAG(isDummy, 1);
        FLAG(oddCycle, 2);

        entitiesService->BindDevice("CPU", &core->m_cpuMem);

        auto tmp = core.get();
        entitiesService->SetProgramCounterInfo("CPU", [tmp](){ return ((WORD) tmp->dmaPage << 8 | tmp->dmaAddr); }, 1);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::FactoryDMA factory;
    return &factory;
}
