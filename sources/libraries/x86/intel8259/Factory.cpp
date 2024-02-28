//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core8259.h"

namespace Astra::CPU::Lib::X86 {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core8259>();

#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))
        REGISTER(m_imr);
        REGISTER(m_irr);
        REGISTER(m_isr);
        REGISTER(m_icwStep);

#define REGISTER2(name, index) entitiesService->AddNewRegister(#name, sizeof(core->m_icw[index]), (void*)&(core->m_icw[index]))

        REGISTER2(icw0, 0);
        REGISTER2(icw1, 1);
        REGISTER2(icw2, 2);
        REGISTER2(icw3, 3);

#define FLAG(name) entitiesService->AddNewFlags(#name, &core->name)

        FLAG(forceInterrupt);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X86::Factory factory;
    return &factory;
}