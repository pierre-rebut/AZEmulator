//
// Created by pierr on 13/01/2024.
//
#include <ctime>
#include "Rtc.h"

#define MHZ 8

#define BCD(a) (((a) / 10) << 4 | ((a) % 10))
#define UNBCD(a) (((a) >> 4) * 10 + ((a) & 0xf))

namespace Astra::CPU::Lib::X16 {

    static const BYTE days_per_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    void Rtc::init(bool set_system_time) {
        vbaten = true;
        h24 = true;
        clocks = 0;

        if (set_system_time) {
            running = true;
            time_t t = time(nullptr);
            struct tm tm = *localtime(&t);
            seconds = tm.tm_sec;
            minutes = tm.tm_min;
            hours = tm.tm_hour;
            day_of_week = (tm.tm_wday == 0 ? 7 : tm.tm_wday);
            day = tm.tm_mday;
            month = tm.tm_mon + 1;
            year = tm.tm_year - 100;
        } else {
            running = false; // yes, the MCP7940N starts out this way!
            seconds = 0;
            minutes = 0;
            hours = 0;
            day_of_week = 1;
            day = 1;
            month = 1;
            year = 0;
        }
    }

    bool Rtc::is_leap_year() const {
        // the clock does 2000-2099, where the "% 4 == 0" rule applies
        return !(year & 3);
    }

    void Rtc::step(int c) {
        if (!running) {
            return;
        }

        clocks += c;
        if (clocks < (MHZ * 1000000)) {
            return;
        }

        clocks -= (MHZ * 1000000);
        seconds++;
        if (seconds < 60) {
            return;
        }

        seconds = 0;
        minutes++;
        if (minutes < 60) {
            return;
        }

        minutes = 0;
        hours++;
        if (hours < 24) {
            return;
        }

        hours = 0;
        day_of_week++;
        if (day_of_week > 7) {
            day_of_week = 1;
        }
        day++;
        BYTE dpm = days_per_month[month - 1];
        if (month == 2 && is_leap_year()) {
            dpm++;
        }
        if (day <= dpm) {
            return;
        }

        day = 1;
        month++;
        if (month <= 12) {
            return;
        }

        month = 1;
        year++;
        if (year == 100) {
            year = 0; // Y2.1K problem! ;-)
        }
    }

    BYTE Rtc::read(BYTE a) const {
        //    printf("RTC READ $%02X\n", a);
        switch (a) {
            case 0:
                return BCD(seconds) | (running << 7);
            case 1:
                return BCD(minutes);
            case 2: {
                BYTE h = hours;
                bool pm = false;
                if (!h24) {
                    // AM/PM
                    if (h >= 12) {
                        pm = true;
                        h -= 12;
                    }
                    if (h == 0) {
                        h = 12;
                    }
                }
                h = BCD(h);
                h |= pm << 5;
                h |= (!h24) << 6;
                return h;
            }
            case 3: {
                BYTE v = day_of_week;
                v |= vbaten << 3;
                v |= running << 5;
                return v;
            }
            case 4:
                return BCD(day);
            case 5:
                return BCD(month) | is_leap_year() << 5;
            case 6:
                return BCD(year);
            default:
                if (a >= 0x20 && a < 0x60) {
                    return nvram[a - 0x20];
                } else if (a >= 0x60) {
                    return 0xff;
                }
                return 0;
        }
    }

    void Rtc::write(BYTE a, BYTE v) {
//    printf("RTC WRITE $%02X, $%02X\n", a, v);
        switch (a) {
            case 0:
                running = !!(v & 0x80);
                seconds = UNBCD(v & 0x7f);
                break;
            case 1:
                minutes = UNBCD(v);
                break;
            case 2: {
                h24 = !(v & 0x40);
                BYTE h = v & 0x3f;
                bool pm = false;
                if (!h24) {
                    pm = v & 0x20;
                    h &= 0x1f;
                }
                h = UNBCD(h);
                if (!h24 && h == 12) {
                    h = 0;
                }
                if (pm) {
                    h += 12;
                }
                hours = h;
                break;
            }
            case 3: {
                day_of_week = v & 7;
                vbaten = !!(v & 0x20);
                break;
            }
            case 4:
                day = UNBCD(v);
                break;
            case 5:
                month = UNBCD(v);
                break;
            case 6:
                year = UNBCD(v);
                break;
            default:
                if (a >= 0x20 && a < 0x60) {
                    nvram[a - 0x20] = v;
                    nvram_dirty = true;
                }
        }
    }
}
