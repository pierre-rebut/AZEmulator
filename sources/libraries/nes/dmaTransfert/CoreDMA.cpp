//
// Created by pierr on 16/10/2023.
//

#include "CoreDMA.h"

namespace Astra::CPU::Lib {

    void CoreDMA::Reset() {
        flags.isTransfer = false;
        flags.isDummy = true;
        flags.oddCycle = false;
        dmaPage = 0;
        dmaAddr = 0;
        dmaData = 0;
        oamAddr = 0;
    }

    void CoreDMA::Execute() {
        if (flags.isTransfer) {

            if (flags.isDummy) {
                if (flags.oddCycle) {
                    flags.isDummy = false;
                    oamAddr = m_cpuMem->Fetch(DataFormat::Byte, 0x2003);
                }
            } else {
                if (!flags.oddCycle) {
                    dmaData = m_cpuMem->Fetch(DataFormat::Byte, (WORD) dmaPage << 8 | dmaAddr);
                } else {
                    m_cpuMem->Push(DataFormat::Byte, 0x2003, dmaAddr);
                    m_cpuMem->Push(DataFormat::Byte, 0x2004, dmaData);

                    dmaAddr++;
                    if (dmaAddr == 0) {
                        flags.isTransfer = false;
                        flags.isDummy = true;
                        flags.oddCycle = false;
                        m_cpuMem->Push(DataFormat::Byte, 0x2003, oamAddr);
                        m_runService->Stop();
                        m_deviceService->SendDeviceRunningStatus(true);
                        return;
                    }
                }
            }

            flags.oddCycle = !flags.oddCycle;
        }
    }

    LARGE CoreDMA::Fetch(DataFormat, size_t address) {
        return 0;
    }

    void CoreDMA::Push(DataFormat, size_t address, LARGE value) {
        flags.isTransfer = true;
        dmaAddr = 0;
        dmaPage = value & 0xFF;

        m_deviceService->SendDeviceRunningStatus(false);
        m_runService->Run();
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{1, 0}};
    const std::vector<std::pair<size_t, size_t>>* CoreDMA::GetDeviceAddressList() const {
        return &addrList;
    }
}
