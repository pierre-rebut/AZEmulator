//
// Created by pierr on 27/02/2024.
//
#pragma once

#include <array>

#include "EngineLib/core/ICpuCore.h"

namespace Astra::CPU::Lib::X686 {

    class CoreCmos : public ICpuCore
    {
    private:
        std::array<BYTE, 128>  ram;
        BYTE  addr, nmi;
        time_t   now;
        int      periodic_ticks, periodic_ticks_max;
        DWORD period;
        int  last_called, uip_period, last_second_update; // todo itick_t

    public:
        bool IsInit() const override {}
        void Reset() override;
        void Execute() override;
    };

}
