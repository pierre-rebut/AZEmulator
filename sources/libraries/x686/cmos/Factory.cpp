//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "CoreCmos.h"

namespace Astra::CPU::Lib::X686 {
    Factory::Factory() : IFactory(RunInfo::STATIC) {

    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreCmos>();

#define REGISTER1(name, index) entitiesService->AddNewRegister(#name, sizeof(core->m_addr[index]), (void*)&(core->m_addr[index]))
#define REGISTER2(name, index) entitiesService->AddNewRegister(#name, sizeof(core->m_cnt[index]), (void*)&(core->m_cnt[index]))
#define FLAG(name, idx)entitiesService->AddNewFlags(#name, &(core->m_flipflop[idx]))

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X686::Factory factory;
    return &factory;
}