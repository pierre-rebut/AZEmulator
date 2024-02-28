//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreBanking.h"

namespace Astra::CPU::Lib::X16 {
    Factory::Factory() : IFactory(RunInfo::STATIC) {
        coreConfig.hardParameters = {"Bank window size", "Address"};
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreBanking>();

#define REGISTER(name, ptr) entitiesService->AddNewRegister(#name, sizeof(core->ptr), (void*)&(core->ptr))
        REGISTER(BankId, m_currentBank);

#define S_REGISTER(name, ptr) entitiesService->AddNewSecondaryRegister(#name, sizeof(core->ptr), (void*)&(core->ptr))
        S_REGISTER(WindowSize, m_bankWindowSize);
        S_REGISTER(MaxNbBank, m_maxNbBank);
        S_REGISTER(RomSize, m_romSize);

        entitiesService->BindDevice("Rom disk", &core->m_romData);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X16::Factory factory;
    return &factory;
}
