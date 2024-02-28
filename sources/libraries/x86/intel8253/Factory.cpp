//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core8253.h"

namespace Astra::CPU::Lib::X86 {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core8253>();
        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X86::Factory factory;
    return &factory;
}