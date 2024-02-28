//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core8237.h"

namespace Astra::CPU::Lib::X86 {
    Factory::Factory() : IFactory(RunInfo::STATIC) {

    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core8237>();

#define REGISTER1(name, index) entitiesService->AddNewRegister(#name, sizeof(core->m_addr[index]), (void*)&(core->m_addr[index]))

        REGISTER1(ADDR0, 0);
        REGISTER1(ADDR1, 1);
        REGISTER1(ADDR2, 2);
        REGISTER1(ADDR3, 3);

#define REGISTER2(name, index) entitiesService->AddNewRegister(#name, sizeof(core->m_cnt[index]), (void*)&(core->m_cnt[index]))

        REGISTER2(CNT0, 0);
        REGISTER2(CNT1, 1);
        REGISTER2(CNT2, 2);
        REGISTER2(CNT3, 3);

#define FLAG(name, idx)entitiesService->AddNewFlags(#name, &(core->m_flipflop[idx]))

        FLAG(flipflop0, 0);
        FLAG(flipflop1, 1);
        FLAG(flipflop2, 2);
        FLAG(flipflop3, 3);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X86::Factory factory;
    return &factory;
}