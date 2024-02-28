//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core8086.h"

namespace Astra::CPU::Lib::CPU8086 {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.debugHeader = {
                "ip", "op", "ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "cs", "ds", "ss", "es", "os", "flags"
        };
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core8086>();
#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(ip);
        REGISTER(sp);
        REGISTER(bp);
        REGISTER(si);
        REGISTER(di);
        REGISTER(cs);
        REGISTER(ss);
        REGISTER(ds);
        REGISTER(es);

#define REGISTER2(name) entitiesService->AddNewRegister(#name, sizeof(core->name.x), (void*)&(core->name.x))

        REGISTER2(ax);
        REGISTER2(bx);
        REGISTER2(cx);
        REGISTER2(dx);

#define S_REGISTER(name) entitiesService->AddNewSecondaryRegister(#name, sizeof(core->name), (void*)&(core->name))

        S_REGISTER(clocks);
        S_REGISTER(os);
        S_REGISTER(d);
        S_REGISTER(w);
        S_REGISTER(mod);
        S_REGISTER(rm);
        S_REGISTER(reg);
        S_REGISTER(rep);
        S_REGISTER(ea);

#define FLAG(name, idx) entitiesService->AddNewFlags(#name, &core->flags, sizeof(core->flags), idx)
        FLAG(CF, 0);
        FLAG(reserved_1, 1);	/* always 1 */
        FLAG(PF, 2);
        FLAG(reserved_2, 3); /* always 0 */
        FLAG(AF, 4);
        FLAG(reserved_3, 5);	/* always 0 */
        FLAG(ZF, 6);
        FLAG(SF, 7);
        FLAG(TF, 8);
        FLAG(IF, 9);
        FLAG(DF, 10);
        FLAG(OF, 11);
        FLAG(IOPL, 12);
        FLAG(IOPH, 13);
        FLAG(NT, 14);
        FLAG(reserved_4, 15);

        entitiesService->BindDevice("Memory", &core->m_mem);
        entitiesService->BindDevice("Devices", &core->m_dev);

        auto tmp = core.get();
        entitiesService->SetProgramCounterInfo("Memory", [tmp]() { return (tmp->cs << 4) + tmp->ip; }, 1);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::CPU8086::Factory factory;
    return &factory;
}