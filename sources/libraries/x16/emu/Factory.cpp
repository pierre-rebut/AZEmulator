//
// Created by pierr on 16/10/2023.
//
#include "Factory.h"
#include "CoreEmu.h"

namespace Astra::CPU::Lib::X16 {
    Factory::Factory() : IFactory(RunInfo::STATIC) {
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<CoreEmu>();

#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(clock_base);
        REGISTER(clock_snap);
        REGISTER(echo_mode);

        entitiesService->AddNewFlags("debugger", &core->debugger_enabled);
        entitiesService->AddNewFlags("video", &core->log_video);
        entitiesService->AddNewFlags("keyboard", &core->log_keyboard);
        entitiesService->AddNewFlags("save", &core->save_on_exit);
        entitiesService->AddNewFlags("cmdkeys", &core->disable_emu_cmd_keys);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X16::Factory factory;
    return &factory;
}
