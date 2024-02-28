//
// Created by pierr on 15/08/2023.
//

#include "AudioDevice.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {

    AudioDevice::AudioDevice(const DeviceCreateData& deviceInfo) : HardwareDevice(deviceInfo.uuid, DeviceType::AUDIO, deviceInfo.name) {
        m_status = DeviceStatus::READY;
        m_masterVolume = deviceInfo.audioData.masterVolume;
    }

    void AudioDevice::Push(DataFormat fmt, size_t address, LARGE value) {
        std::scoped_lock guard(m_mtx);

        m_sampleQueue.push(value);
    }

    static const std::vector<std::pair<size_t, size_t>> audioAddrList = {{sizeof(LARGE), 0}};

    const std::vector<std::pair<size_t, size_t>>& AudioDevice::GetDeviceAddressList() const {
        return audioAddrList;
    }

    static float clip(float fSample, float fMax) {
        if (fSample >= 0.0)
            return std::min(fSample, fMax);
        else
            return std::max(fSample, -fMax);
    };

    void AudioDevice::PopSampleQueue(float* sampleStream, int sampleNb) {
        float lastSampleValue = 0;
        int i = 0;

        {
            std::scoped_lock guard(m_mtx);

            while (i < sampleNb && !m_sampleQueue.empty()) {
                auto value = *(double*) &m_sampleQueue.front();

                lastSampleValue = clip((float) value, 1);
                m_sampleQueue.pop();

                sampleStream[i] = lastSampleValue;
                i++;
            }
        }

        while (i < sampleNb) {
            sampleStream[i] = lastSampleValue;
            i++;
        }
    }
}
