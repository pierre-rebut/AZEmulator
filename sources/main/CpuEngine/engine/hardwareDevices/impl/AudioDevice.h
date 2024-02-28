//
// Created by pierr on 15/08/2023.
//
#pragma once

#include <queue>

#include "CpuEngine/data/DeviceCreateData.h"
#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"

namespace Astra::CPU::Core {

    class AudioDevice : public HardwareDevice
    {
    private:
        std::queue<LARGE> m_sampleQueue;
        float m_masterVolume = 1;

    public:
        explicit AudioDevice(const DeviceCreateData& deviceInfo);

        void Push(DataFormat fmt, size_t address, LARGE value) override;
        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override;

        void PopSampleQueue(float* sampleStream, int sampleNb);

        void SetMasterVolume(float v) {m_masterVolume = v;}
        float GetMasterVolume() const {return m_masterVolume;}
    };

}
