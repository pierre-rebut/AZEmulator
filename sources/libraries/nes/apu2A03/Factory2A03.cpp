//
// Created by pierr on 18/10/2023.
//
#include "Factory2A03.h"
#include "Core2A03.h"

namespace Astra::CPU::Lib {
    Factory2A03::Factory2A03() : IFactory(RunInfo::RUNNABLE) {
    }

    Scope<ICpuCore> Factory2A03::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core2A03>();
#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(frame_clock_counter);
        REGISTER(clock_counter);
        REGISTER(pulse1_visual);
        REGISTER(pulse2_visual);
        REGISTER(noise_visual);
        REGISTER(triangle_visual);
        REGISTER(dAudioSample);

#define FLAG(name, idx)entitiesService->AddNewFlags(#name, &core->flags.reg, sizeof(core->flags.reg), idx);

        FLAG(bUseRawMode, 0);
        FLAG(pulse1_enable, 1);
        FLAG(pulse1_halt, 2);
        FLAG(pulse2_enable, 3);
        FLAG(pulse2_halt, 4);
        FLAG(noise_enable, 5);
        FLAG(noise_halt, 6);
        FLAG(bAudioSampleReady, 7);

        entitiesService->BindDevice("Audio Out", &core->m_audioOut);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Factory2A03 factory;
    return &factory;
}