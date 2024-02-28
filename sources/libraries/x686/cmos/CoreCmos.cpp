//
// Created by pierr on 27/02/2024.
//
#include "CoreCmos.h"

#define FREQUENCY          32768
#define ALARM_SEC          1
#define ALARM_MIN          3
#define ALARM_HOUR         5

#define is24hour() (ram[0x0B] & 2)
#define PERIODIC   0x40
#define ALARM      0x20
#define UPDATE     0x10

namespace Astra::CPU::Lib::X686 {
    void CoreCmos::Reset() {
        ram[0x0A] = 0x26;
        ram[0x0B] = 0x02;
        ram[0x0C] = 0x00;
        ram[0x0D] = 0x80;
    }

    void CoreCmos::Execute() {

        int next = last_called + period;
        if (now >= next) {
            int why = 0;
            if (ram[0x0B] & 0x40) {
                why |= PERIODIC;
                periodic_ticks++;
                if (periodic_ticks != periodic_ticks_max)
                    goto done;
                periodic_ticks = 0;
            }
            now++;
            if (ram[0x0B] & 0x20) {
                int ok = 1;
                ok &= cmos_ram_read(ALARM_SEC) == cmos_ram_read(0);
                ok &= cmos_ram_read(ALARM_MIN) == cmos_ram_read(2);
                ok &= cmos_ram_read(ALARM_HOUR) == cmos_ram_read(4);
                if (ok)
                    why |= ALARM;
            }
            if (ram[0x0B] & 0x10) {
                why |= UPDATE;
            }
            last_second_update = now;
            done:
            last_called = get_now();
            if (why) {
                cmos_raise_irq(why);
                return 1;
            }
        }
        return 0;
    }

}
