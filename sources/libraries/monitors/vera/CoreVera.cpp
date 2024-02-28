//
// Created by pierr on 16/10/2023.
//

#include "CoreVera.h"

#define CLOCKS 6

namespace Astra::CPU::Lib::Monitors {

    void CoreVera::Reset() {
        spi.reset();
        vera.reset();
        m_isInit = true;
    }

    void CoreVera::Execute() {
        spi.step(CLOCKS);

        bool newFrame = vera.step(8, CLOCKS, false);
        if (newFrame) {
            vera.update();

            auto framebuffer = vera.GetFrameBuffer();
            for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
                auto addrSrc = i * 4;
                auto addrDst = i * 3;

                using
                enum Astra::CPU::DataFormat;
                m_video->Push(Byte, addrDst + 0, framebuffer[addrSrc + 2]);
                m_video->Push(Byte, addrDst + 1, framebuffer[addrSrc + 1]);
                m_video->Push(Byte, addrDst + 2, framebuffer[addrSrc + 0]);
            }
        }

        if (vera.isIrq()) {
            m_deviceService->SendDeviceInterrupt(1);
        }
    }

    LARGE CoreVera::Fetch(DataFormat, size_t address) {
        return vera.read((BYTE) address, false);
    }

    void CoreVera::Push(DataFormat, size_t address, LARGE value) {
        vera.write((BYTE) address, (BYTE) value);
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{32, 0}};

    const std::vector<std::pair<size_t, size_t>>* CoreVera::GetDeviceAddressList() const {
        return &addrList;
    }

    bool CoreVera::UpdateHardParameters(const std::vector<int>& hardParameters) {
        CoreVera::Reset();
        return true;
    }
}
