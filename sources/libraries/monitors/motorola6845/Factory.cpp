//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core6845.h"

namespace Astra::CPU::Lib::Monitors {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {

    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core6845>();

#define REGISTER(name, val) entitiesService->AddNewRegister(#name, sizeof(core->val), (void*)&(core->val))
        REGISTER(index, m_index);
        REGISTER(retrace, m_retrace);
        REGISTER(vShift, m_vShift);

#define REGISTER2(name, index) entitiesService->AddNewRegister(#name, sizeof(core->m_registers[index]), (void*)&(core->m_registers[index]))

        REGISTER2(R0, 0);
        REGISTER2(R1, 1);
        REGISTER2(R2, 2);
        REGISTER2(R3, 3);
        REGISTER2(R4, 4);
        REGISTER2(R5, 5);
        REGISTER2(R6, 6);
        REGISTER2(R7, 7);
        REGISTER2(R8, 8);
        REGISTER2(R9, 9);
        REGISTER2(RA, 10);
        REGISTER2(RB, 11);
        REGISTER2(RC, 12);
        REGISTER2(RD, 13);
        REGISTER2(RE, 14);
        REGISTER2(RF, 15);

        entitiesService->BindDevice("Video", &core->m_video);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Monitors::Factory factory;
    return &factory;
}