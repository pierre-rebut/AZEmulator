//
// Created by pierr on 16/10/2023.
//

#include "CoreScreen.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::Monitors {

    static const int WIDTH = 128;
    static const int HEIGHT = 64;

    void CoreScreen::Push(DataFormat, size_t address, LARGE value) {
        if (address >= WIDTH * HEIGHT) {
            return;
        }

        address *= 3;

        static const BYTE tmp[] = {0, 64, 128, 255};

        m_video->Push(DataFormat::Byte, address + 0, tmp[(value & 0b00000011)]);
        m_video->Push(DataFormat::Byte, address + 1, tmp[(value & 0b00001100) >> 2]);
        m_video->Push(DataFormat::Byte, address + 2, tmp[(value & 0b00110000) >> 4]);
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{WIDTH * HEIGHT, 0}};
    const std::vector<std::pair<size_t, size_t>>* CoreScreen::GetDeviceAddressList() const {
        return &addrList;
    }
}
