//
// Created by pierr on 16/10/2023.
//

#include <iostream>

#include "CoreEmu.h"

namespace Astra::CPU::Lib::X16 {

    static const std::vector<std::pair<size_t, size_t>> addrList = {{160, 0}};

    const std::vector<std::pair<size_t, size_t>>* CoreEmu::GetDeviceAddressList() const {
        return &addrList;
    }

    void CoreEmu::Push(DataFormat, size_t address, LARGE value) {
        if (address < 80 || address > 95) {
            return;
        }
        address &= 0xf;

        bool v = value != 0;
        switch (address) {
            case 0: debugger_enabled = v; break;
            case 1: log_video = v; break;
            case 2: log_keyboard = v; break;
            case 3: echo_mode = value; break;
            case 4: save_on_exit = v; break;
            case 7: disable_emu_cmd_keys = v; break;
            case 8: clock_base = m_runService->GetSystemTicks(); break;
            case 9: std::cout << std::hex << "User debug 1: $" << value << std::endl; break;
            case 10: std::cout << std::hex << "User debug 2: $" << value << std::endl; break;
            default:
                break;
        }
    }

    LARGE CoreEmu::Fetch(DataFormat fmt, size_t address) {
        if (address < 80 || address > 95) {
            return 0x9f;
        }

        address &= 0xf;

        switch (address) {
            case 0:
                return debugger_enabled ? 1 : 0;
            case 1:
                return log_video ? 1 : 0;
            case 2:
                return log_keyboard ? 1 : 0;
            case 3:
                return echo_mode;
            case 4:
                return save_on_exit ? 1 : 0;
            case 5:
                return 0; // record gif disable
            case 7:
                return disable_emu_cmd_keys ? 1 : 0;
            case 8:
                clock_snap = m_runService->GetSystemTicks() - clock_base;
            case 9:
                return (clock_snap >> 8) & 0xff;
            case 10:
                return (clock_snap >> 16) & 0xff;
            case 11:
                return (clock_snap >> 24) & 0xff;
            case 13:
                return 15; // keymap en
            case 14:
                return '1'; // emulator detection
            case 15:
                return '6'; // emulator detection
            default:
                return -1;
        }
    }

    int CoreEmu::FetchReadOnly(size_t address) const {
        if (address < 80 || address > 95) {
            return 0x9f;
        }

        address &= 0xf;

        switch (address) {
            case 0:
                return debugger_enabled ? 1 : 0;
            case 1:
                return log_video ? 1 : 0;
            case 2:
                return log_keyboard ? 1 : 0;
            case 3:
                return echo_mode;
            case 4:
                return save_on_exit ? 1 : 0;
            case 5:
                return 0; // record gif disable
            case 7:
                return disable_emu_cmd_keys ? 1 : 0;
            case 9:
                return (clock_snap >> 8) & 0xff;
            case 10:
                return (clock_snap >> 16) & 0xff;
            case 11:
                return (clock_snap >> 24) & 0xff;
            case 13:
                return 15; // keymap fr
            case 14:
                return '1'; // emulator detection
            case 15:
                return '6'; // emulator detection
            default:
                return -1;
        }
    }

    void CoreEmu::Reset() {
        debugger_enabled = false;
        disable_emu_cmd_keys = false;
        log_video = false;
        log_keyboard = false;
        echo_mode = 0;
        save_on_exit = false;

        clock_base = 0;
        clock_snap = 0;
    }
}
