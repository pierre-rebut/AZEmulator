//
// Created by pierr on 13/01/2024.
//
#pragma once

#include "EngineLib/data/Types.h"

namespace Astra::CPU::Lib::X16 {

    class Rtc
    {
    private:
        bool nvram_dirty = false;
        BYTE nvram[0x40];

        bool running;
        bool vbaten;
        bool h24;

        unsigned int clocks;
        int seconds;
        int minutes;
        int hours;
        int day_of_week;
        int day;
        int month;
        int year;

    public:
        void init(bool set_system_time);
        void step(int c);
        BYTE read(BYTE a) const;
        void write(BYTE a, BYTE v);

    private:
        bool is_leap_year() const;
    };

}
