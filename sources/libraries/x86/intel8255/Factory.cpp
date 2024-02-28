//
// Created by pierr on 22/10/2023.
//
#include "Factory.h"
#include "Core8255.h"

namespace Astra::CPU::Lib::X86 {
    Factory::Factory() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore> Factory::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core8255>();

#define FLAG(name, idx) entitiesService->AddNewFlags(#name, &core->status.reg, sizeof(core->status.reg), idx);

        FLAG(OutputBufferFull, 0);
        FLAG(InputBufferFull, 1);
        FLAG(SystemFlag, 2);
        FLAG(CommandData, 3);
        FLAG(KeyboardInhibit, 4);
        FLAG(AuxiliaryDeviceOutputBuffer, 5);
        FLAG(GeneralPurposeTimeOut, 6);
        FLAG(ParityError, 7);

        entitiesService->BindDevice("Keyboard", &core->m_keyboard);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::X86::Factory factory;
    return &factory;
}