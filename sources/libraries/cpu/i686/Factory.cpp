//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core686.h"

namespace Astra::CPU::Lib::CPU {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.debugHeader = {
                "ip", "op", "ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "cs", "ds", "ss", "es", "os", "flags"
        };
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core686>();

#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))
#define REGISTER2(name) entitiesService->AddNewRegister(#name, sizeof(core->name.x), (void*)&(core->name.x))
#define S_REGISTER(name) entitiesService->AddNewSecondaryRegister(#name, sizeof(core->name), (void*)&(core->name))
#define FLAG(name, idx) entitiesService->AddNewFlags(#name, &core->flags, sizeof(core->flags), idx)

        entitiesService->BindDevice("Memory", &core->m_ram);
        entitiesService->BindDevice("Devices", &core->m_ioBus);
        entitiesService->BindDevice("MMIO", &core->m_mmio);

        auto tmp = core.get();
        //entitiesService->SetProgramCounterInfo("Memory", [tmp]() { return (tmp->cs << 4) + tmp->ip; }, 1);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::CPU::Factory factory;
    return &factory;
}