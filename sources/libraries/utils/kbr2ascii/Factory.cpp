//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreKbr2Ascii.h"

namespace Astra::CPU::Lib::Utils {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.hardParameters = {
                "Always uppercase"
        };
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreKbr2Ascii>();

        entitiesService->AddNewFlags("Shift", &core->m_shift);
        entitiesService->AddNewFlags("Ctrl", &core->m_ctrl);
        entitiesService->AddNewFlags("Upper", &core->m_onlyUpper);

        entitiesService->BindDevice("Keyboard", &core->m_keyboard);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Utils::Factory factory;
    return &factory;
}
