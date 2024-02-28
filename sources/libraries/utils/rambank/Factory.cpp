//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreBanking.h"

namespace Astra::CPU::Lib::Utils {
    Factory::Factory() : IFactory(RunInfo::STATIC) {
        coreConfig.hardParameters = {"Bank window size", "MaxBankId", "Address"};
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreBanking>();

#define REGISTER(name, ptr) entitiesService->AddNewRegister(#name, sizeof(core->ptr), (void*)&(core->ptr))
        REGISTER(BankId, m_currentBank);
        REGISTER(WindowSize, m_bankWindowSize);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Utils::Factory factory;
    return &factory;
}
