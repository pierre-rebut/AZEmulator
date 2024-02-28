//
// Created by pierr on 20/10/2023.
//
#include "FactoryCart.h"
#include "CoreCart.h"

namespace Astra::CPU::Lib {
    FactoryCart::FactoryCart() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> FactoryCart::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreCart>();

#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))
        REGISTER(nMapperID);
        REGISTER(nPRGBanks);
        REGISTER(nCHRBanks);
        REGISTER(hw_mirror);

#define FLAG(name, idx)entitiesService->AddNewFlags(#name, &core->flags.reg, sizeof(core->flags.reg), idx);
        FLAG(isImageReady, 0);

        entitiesService->BindDevice("CartSlot", &core->m_cartSlot);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::FactoryCart factory;
    return &factory;
}
