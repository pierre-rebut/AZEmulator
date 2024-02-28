//
// Created by pierr on 16/10/2023.
//

#include "CoreVia.h"
#include "EngineLib/data/KeyCodes.h"

#define CLOCKS 6

namespace Astra::CPU::Lib::X16 {

    CoreVia::CoreVia() {
        CoreVia::Reset();
    }

    void CoreVia::Reset() {
        via[0].init();
        via[1].init();

        smc.reset();
        rtc.init(set_system_time);

        i2c.resetState();
        serial.reset();
    }
    static const std::vector<std::pair<size_t, size_t>> addrList = {{32,0}};

    const std::vector<std::pair<size_t, size_t>>* CoreVia::GetDeviceAddressList() const {
        return &addrList;
    }

    bool CoreVia::UpdateHardParameters(const std::vector<int>& hardParameters) {
        set_system_time = hardParameters.at(0);
        return true;
    }

    void CoreVia::Execute() {
        smc.step();

        via[0].step(CLOCKS);
        via[1].step(CLOCKS);

        //serial.step(CLOCKS);

        for (int i = 0; i < CLOCKS; i++) {
            i2c.step();
        }

        rtc.step(CLOCKS);

        if (sendNmi) {
            m_deviceService->SendDeviceNonMaskableInterrupt(0);
            sendNmi = false;
        }

        if (via[0].isIrq() || via[1].isIrq()) {
            m_deviceService->SendDeviceInterrupt(1);
        }
    }

    LARGE CoreVia::Fetch(DataFormat fmt, size_t address) {
        if (address < 16) {
            return readVia1(address & 0xf, false);
        }
        return readVia2(address & 0xf, false);
    }

    void CoreVia::Push(DataFormat fmt, size_t address, LARGE value) {
        if (address < 16) {
            writeVia1(address & 0xf, value);
        } else {
            writeVia2(address & 0xf, value);
        }
    }

    BYTE CoreVia::readVia1(BYTE reg, bool debug) {
        // DDR=0 (input)  -> take input bit
        // DDR=1 (output) -> take output bit
        // For now, just assume that I2C peripherals always drive all lines and VIA
        // overrides them with their input values based on current DDR bits
        switch (reg) {
            case 0: // PB
                if (!debug) via[0].clearPrbIrqs();
                if (via[0].Get(11) & 2) {
                    // TODO latching mechanism (requires IEC implementation)
                    return 0;
                } else {
                    return (~via[0].Get(2) & (
                                    (serial.readClk() << 6) |
                                    (serial.readData() << 7)
                            )) |
                            (via[0].Get(2) & (
                                    (serial.in.atn << 3) |
                                    ((!serial.in.clk) << 4) |
                                    ((!serial.in.data) << 5)
                            ));
                }

            case 1: // PA
            case 15:
                i2c.step();
                if (!debug) via[0].clearPraIrqs();
                if (via[0].Get(11) & 1) {
                    // CA1 is currently not connected to anything (?)
                    return 0;
                } else {
                    return (~via[0].Get(3) & i2c.port.data_out) |
                           //I2C Data: PA0=1 if DDR bit is 0 (input) and data_out is 1; usage of data_out and data_in is a bit confusing...
                           (via[0].Get(3) & i2c.port.data_in) |                        //I2C Data: PA0=1 if DDR bit is 1 (output) and data_in is 1
                           (~via[0].Get(3) & I2C_CLK_MASK) |
                           //I2C Clock: PA1=1 if DDR bit is 0 (input), simulating an input pull-up
                           (via[0].Get(3) & i2c.port.clk_in);
                           //I2C Clock: PA1=1 if DDR bit is 1 (output) and clk_in is 1, simulating a pin driven by the VIA
                           //Joystick_data;
                }

            default:
                return via[0].read(reg, debug);
        }
    }

    void CoreVia::writeVia1(BYTE reg, BYTE value) {
        via[0].write(reg, value);
        if (reg == 0 || reg == 2) {
            // PB
            const BYTE pb = via[0].Get(0) | ~via[0].Get(2);
            serial.in.atn = (pb & Serial::ATNIN_MASK) != 0;
            serial.in.clk = (pb & Serial::CLOCKIN_MASK) == 0;
            serial.in.data = (pb & Serial::DATAIN_MASK) == 0;

        } else if (reg == 1 || reg == 3) {
            i2c.step();
            // PA
            const BYTE pa = via[0].Get(1) | ~via[0].Get(3);
            i2c.port.data_in = pa & I2C_DATA_MASK;                                        //Sets data_in = 1 if the corresponding DDR bit is 0 (input), simulates a pull-up
            i2c.port.clk_in = (pa & I2C_CLK_MASK) >> 1;                                    //Sets clk_in = 1 if pin is an input, simulates a pull-up
            //joystick_set_latch(via[0].Get(1) & JOY_LATCH_MASK);
            //joystick_set_clock(via[0].Get(1) & JOY_CLK_MASK);
        }
    }

/*
** VIA#2
**
** PA/PB: user port
** for now, just assume that all user ports are not connected
** and reads return output register (open bus behavior)
*/

    BYTE CoreVia::readVia2(BYTE reg, bool debug) {
        return via[1].read(reg, debug);
    }

    void CoreVia::writeVia2(BYTE reg, BYTE value) {
        via[1].write(reg, value);
    }
}
