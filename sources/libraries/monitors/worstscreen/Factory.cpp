//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreScreen.h"

namespace Astra::CPU::Lib::Monitors {
    Factory::Factory() : IFactory(RunInfo::STATIC) {
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreScreen>();

        entitiesService->BindDevice("Video", &core->m_video);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Monitors::Factory factory;
    return &factory;
}
