//
// Created by pierr on 15/10/2023.
//

#include "Factory2C02.h"
#include "Core2C02.h"

namespace Astra::CPU::Lib {
    Factory2C02::Factory2C02() : IFactory(RunInfo::RUNNABLE) {
        coreConfig.allowDeviceConnection = true;
    }

    Scope<ICpuCore>
    Factory2C02::CreateNewCore(IEntitiesService* entitiesService) const {
        auto core = std::make_unique<Core2C02>();
#define REGISTER(name) entitiesService->AddNewRegister(#name, sizeof(core->name), (void*)&(core->name))

        REGISTER(fine_x);
        REGISTER(address_latch);
        REGISTER(ppu_data_buffer);
        REGISTER(scanline);
        REGISTER(cycle);
        REGISTER(bg_next_tile_id);
        REGISTER(bg_next_tile_attrib);
        REGISTER(bg_next_tile_lsb);
        REGISTER(bg_next_tile_msb);
        REGISTER(bg_shifter_pattern_lo);
        REGISTER(bg_shifter_pattern_hi);
        REGISTER(bg_shifter_attrib_lo);
        REGISTER(bg_shifter_attrib_hi);

#define REGISTER2(name) entitiesService->AddNewRegister(#name, sizeof(core->name.reg), (void*)&(core->name.reg))
        REGISTER2(status);
        REGISTER2(mask);
        REGISTER2(control);
        REGISTER2(vram_addr);
        REGISTER2(tram_addr);

#define FLAG(name, index) entitiesService->AddNewFlags(#name, &core->flags.reg, sizeof(core->flags.reg), index);
        FLAG(frame_complete, 0)
        FLAG(scanline_trigger, 1)
        FLAG(odd_frame, 2)
        FLAG(bSpriteZeroHitPossible, 3)
        FLAG(bSpriteZeroBeingRendered, 4)

        entitiesService->BindDevice("Video", &core->m_video);
        entitiesService->BindDevice("VRam", &core->m_vram);

        return core;
    }
}

extern "C" Astra::CPU::IFactory* CoreFactory() {
    static Astra::CPU::Lib::Factory2C02 factory;
    return &factory;
}