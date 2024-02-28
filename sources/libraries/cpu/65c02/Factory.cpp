//
// Created by pierr on 19/10/2023.
//

#include "Factory.h"
#include "Core65c02.h"

namespace Astra::CPU::Lib::CPU {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.debugHeader = {
            "pc", "op", "a", "x", "y", "stkp", "flags"
        };
        coreConfig.hardParameters = {"Is 65c02 ?"};
    }

    Scope<ICpuCore>
    Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core65c02>();
#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(pc);
        REGISTER(stkp);
        REGISTER(a);
        REGISTER(x);
        REGISTER(y);

#define S_REGISTER(name) entitiesService->AddNewSecondaryRegister(#name, sizeof(core->name), (void*)&(core->name))

        S_REGISTER(fetched);
        S_REGISTER(temp);
        S_REGISTER(addr_abs);
        S_REGISTER(addr_rel);
        S_REGISTER(opcode);
        S_REGISTER(cycles);

#define FLAG(name, idx) entitiesService->AddNewFlags(#name, &core->status, sizeof(core->status), idx);

        FLAG(C, 0);
        FLAG(Z, 1);
        FLAG(I, 2);
        FLAG(D, 3);
        FLAG(B, 4);
        FLAG(U, 5);
        FLAG(V, 6);
        FLAG(N, 7);

        entitiesService->BindDevice("Main", &core->m_mem);

        auto tmp = core.get();
        entitiesService->SetProgramCounterInfo("Main", [tmp]() { return (tmp->pc); }, 1);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::CPU::Factory factory;
    return &factory;
}
