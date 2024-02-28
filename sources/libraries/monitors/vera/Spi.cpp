//
// Created by pierr on 17/01/2024.
//
#include "Spi.h"

namespace Astra::CPU::Lib::Monitors {
    void Spi::reset() {
        ss = false;
        busy = false;
        autotx = false;
        received_byte = 0xff;
        sdCard.reset();
    }

    void Spi::step(int clocks) {
        if (busy) {
            outcounter += clocks;
            if (outcounter >= 8) {
                busy = false;
                if (sdCard.isAttached()) {
                    received_byte = sdCard.handle(sending_byte);
                } else {
                    received_byte = 0xff;
                }
            }
        }
    }

    BYTE Spi::read(BYTE reg) {
        switch (reg) {
            case 0:
                if (autotx && ss && !busy) {
                    // autotx mode will automatically send $FF after each read
                    sending_byte = 0xff;
                    busy = true;
                    outcounter = 0;
                }
                return received_byte;
            case 1:
                return busy << 7 | autotx << 3 | ss;
        }
        return 0;
    }

    void Spi::write(BYTE reg, BYTE value) {
        switch (reg) {
            case 0:
                if (ss && !busy) {
                    sending_byte = value;
                    busy = true;
                    outcounter = 0;
                }
                break;
            case 1:
                if (ss != (value & 1)) {
                    ss = value & 1;
                    if (ss) {
                        sdCard.select(ss);
                    }
                }
                autotx = !!(value & 8);
                break;
        }
    }
}
