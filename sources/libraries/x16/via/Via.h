//
// Created by pierr on 13/01/2024.
//
#pragma once

#include "EngineLib/data/Types.h"

namespace Astra::CPU::Lib::X16 {

    class Via
    {
    private:
        unsigned timerCount[2];
        unsigned pb6PulseCounts;
        BYTE registers[15];
        bool timer1M1;
        bool timerRunning[2];
        bool pb7Output;

    public:
        void init();
        void clearPraIrqs();
        void clearPrbIrqs();
        BYTE read(BYTE reg, bool debug);
        void write(BYTE reg, BYTE value);
        void step(unsigned clocks);
        bool isIrq() const;

        BYTE Get(int id) const {return registers[id];}
    };

}
