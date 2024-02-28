//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreYamaha.h"

namespace Astra::CPU::Lib::X16 {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreYamaha>();

#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(addr_ym);

        entitiesService->BindDevice("Audio", &core->m_audioOut);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X16::Factory factory;
    return &factory;
}
