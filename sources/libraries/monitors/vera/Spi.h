//
// Created by pierr on 17/01/2024.
//
#pragma once

#include "EngineLib/data/Types.h"
#include "SdCard.h"

namespace Astra::CPU::Lib::Monitors {

    class Spi
    {
    private:
        bool ss = false;
        bool busy = false;
        bool autotx = false;
        BYTE sending_byte = 0xff;
        BYTE received_byte = 0xff;
        int outcounter = 0;

        SdCard sdCard;

    public:
        explicit Spi(X16Disk& sd) : sdCard(sd) {}

        void reset();
        void step(int clocks);
        BYTE read(BYTE reg);
        void write(BYTE reg, BYTE value);
    };

}
