//
// Created by pierr on 14/01/2024.
//
#pragma once

#include "EngineLib/data/Types.h"

namespace Astra::CPU::Lib::X16 {

    class Serial
    {
    public:
        static constexpr const int ATNIN_MASK =  1 << 3;
        static constexpr const int CLOCKIN_MASK =  1 << 4;
        static constexpr const int DATAIN_MASK =  1 << 5;

        struct {
            int atn;
            int clk;
            int data;
        } in;
        struct {
            int clk;
            int data;
        } out;

    private:
        int state = 0;
        bool valid;
        int bit;
        BYTE byte;
        bool listening = false;
        bool talking = false;
        bool during_atn = false;
        bool eoi = false;
        bool fnf = false; // file not found
        int clocks_since_last_change = 0;

        bool old_atn = false, old_clk = false, old_data = false;

    public:
        void reset();
        void step(int clocks);
        bool readClk() const;
        bool readData() const;
    };

}
