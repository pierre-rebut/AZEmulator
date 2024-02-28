//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/KeyCodes.h"
#include "EngineLib/data/Base.h"
#include "ym2151Interface.h"

#define SAMPLES_PER_BUFFER (256)
#define SAMP_POS_FRAC_BITS (24)

namespace Astra::CPU::Lib::X16 {

    class CoreYamaha : public ICpuCore
    {
    private:
        friend class Factory;

        BYTE addr_ym = 0;

        ym2151Interface opmIface;

        int16_t * buffer = nullptr;
        DWORD buffer_size = 0;
        DWORD rdidx = 0;
        DWORD wridx = 0;
        DWORD buffer_written = 0;
        DWORD vera_samp_pos_rd = 0;
        DWORD vera_samp_pos_wr = 0;
        DWORD vera_samp_pos_hd = 0;
        DWORD ym_samp_pos_rd = 0;
        DWORD ym_samp_pos_wr = 0;
        DWORD ym_samp_pos_hd = 0;
        DWORD vera_samps_per_host_samps = 0;
        DWORD ym_samps_per_host_samps = 0;
        DWORD limiter_amp = 0;

        int16_t psg_buf[2 * SAMPLES_PER_BUFFER];
        int16_t pcm_buf[2 * SAMPLES_PER_BUFFER];
        int16_t ym_buf[2 * SAMPLES_PER_BUFFER];

        DWORD host_sample_rate = 0;

        int clockTickFix = 0;

        Ref<IDevice> m_audioOut;

    public:
        CoreYamaha();
        ~CoreYamaha() override;
        bool IsInit() const override { return m_audioOut && buffer; }

        void Execute() override;
        void Reset() override;
        void Interrupt(bool isNmi, int interruptId) override;

        void Push(DataFormat, size_t address, LARGE value) override;
        LARGE Fetch(DataFormat fmt, size_t address) override;
        int FetchReadOnly(size_t address) const override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

    private:
        void audio_render();
        void sendSampleToAudioDevice();
    };

}
