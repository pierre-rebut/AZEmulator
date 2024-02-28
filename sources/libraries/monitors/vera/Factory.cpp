//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreVera.h"

namespace Astra::CPU::Lib::Monitors {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreVera>();

        entitiesService->BindDevice("Video", &core->m_video);
        entitiesService->BindDevice("SdCard", &core->m_sdCard);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Monitors::Factory factory;
    return &factory;
}
