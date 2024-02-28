//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreVia.h"

namespace Astra::CPU::Lib::X16 {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.hardParameters = {"SetSystemTime"};
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreVia>();

        entitiesService->AddNewFlags("SetSystemTime", &core->set_system_time);

        entitiesService->BindDevice("Keyboard", &core->m_keyboard);
        entitiesService->BindDevice("Mouse", &core->m_mouse);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X16::Factory factory;
    return &factory;
}
