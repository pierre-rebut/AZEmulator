//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreGamepad.h"

namespace Astra::CPU::Lib::Nes {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreGamepad>();
#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(m_controller);
        REGISTER(m_controllerState);

        entitiesService->BindDevice("Keyboard", &core->m_keyboard);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Nes::Factory factory;
    return &factory;
}
