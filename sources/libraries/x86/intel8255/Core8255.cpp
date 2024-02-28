//
// Created by pierr on 27/10/2023.
//
#include "Core8255.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::X86 {
    void Core8255::Reset() {
        status.reg = 0;
        ports[0] = 0x2C;
    }

    void Core8255::Execute() {
        if (const DWORD keyCode = m_keyboard->Fetch(DataFormat::DWord, 0)) {
            const bool isKeyDown = keyCode >> 16;
            int x86KeyCode = keyCodeTox86KeyCode((KeyCode) (keyCode & 0xFFFF));

            if (!isKeyDown) {
                x86KeyCode |= 1 << 7;
            }

            ports[0] = x86KeyCode;
            m_deviceService->SendDeviceInterrupt(1);
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{4, 0}};

    const std::vector<std::pair<size_t, size_t>>* Core8255::GetDeviceAddressList() const {
        return &addrList;
    }

    LARGE Core8255::Fetch(DataFormat, size_t address) {
        if (address == 0b11) {
            return status.reg;
        }

        return ports[address & 0b11];
    }

    void Core8255::Push(DataFormat, size_t address, LARGE val) {
        if (address == 0b11) {
            status.reg = val & 0xFF;
        } else {
            ports[address & 0b11] = (int) val;
        }
    }

    int Core8255::keyCodeTox86KeyCode(KeyCode keyCode) {
        switch (keyCode) {
            case KeyCode::D0:
                return 79;
            default:
                return 0;
        }
    }
}
