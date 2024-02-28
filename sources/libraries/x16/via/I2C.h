//
// Created by pierr on 13/01/2024.
//
#pragma once

#include "EngineLib/data/Types.h"
#include "Smc.h"
#include "Rtc.h"

namespace Astra::CPU::Lib::X16 {

static constexpr const int DEVICE_SMC = 0x42;
static constexpr const int DEVICE_RTC = 0x6F;

static constexpr const int STATE_START = 0;
static constexpr const int STATE_STOP = -1;

static constexpr const int I2C_DATA_MASK = 1;
static constexpr const int I2C_CLK_MASK = 2;

    class I2C
    {
    public:
        struct I2cPort {
            int clk_in;
            int data_in;
            int data_out;
        };

        I2cPort port;
        I2cPort oldPort;

        Smc& smc;
        Rtc& rtc;

    private:
        int state = STATE_STOP;
        bool read_mode = false;
        BYTE value = 0;
        int count = 0;
        BYTE device;
        BYTE offset;

    public:
        I2C(Smc& a, Rtc& b) : smc(a), rtc(b) {}

        void resetState();
        BYTE read(BYTE device, BYTE offset);
        void write(BYTE device, BYTE offset, BYTE value);
        void step();
    };

}
