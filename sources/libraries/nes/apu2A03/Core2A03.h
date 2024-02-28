//
// Created by pierr on 18/10/2023.
//
#pragma once

#include <functional>

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib {

    class Core2A03  : public ICpuCore
    {
    private:
        friend class Factory2A03;

        uint32_t frame_clock_counter = 0;
        uint32_t clock_counter = 0;

        double dAudioSample = 0.0;
        double dAudioTime = 0.0;
        double dAudioTimePerNESClock = 0.0;
        double dAudioTimePerSystemSample = 0.0f;

        Ref<IDevice> m_audioOut = nullptr;

        union Flags {
            struct {
                bool bUseRawMode : 1 = false;
                bool pulse1_enable : 1 = false;
                bool pulse1_halt : 1 = false;
                bool pulse2_enable : 1 = false;
                bool pulse2_halt : 1 = false;
                bool noise_enable : 1 = false;
                bool noise_halt : 1 = false;
                bool bAudioSampleReady : 1 = false;
            };

            BYTE reg;
        } flags;


        static uint8_t length_table[];

        // Sequencer Module
        // ~~~~~~~~~~~~~~~~
        // The purpose of the sequencer is to output a '1' after a given
        // interval. It does this by counting down from a start value,
        // when that value is < 0, it gets Reset, and an internal "rotary"
        // buffer is shifted. The nature of ths shifted pattern is different
        // depending upon the channel, or module that requires sequencing.
        // For example, the square wave channels simply rotate the preset
        // sequence, but the noise channel needs to generate pseudo-random
        // outputs originating from the preset sequence.
        //
        // Consider a square wave channel. A preset sequence of 01010101
        // will output a 1 more freqently than 00010001, assuming we
        // always output the LSB. The speed of this output is also
        // governed by the timer counting down. The frequency is higher
        // for small timer values, and lower for larger. Increasing
        // the frequency of the output potentially increases the
        // audible frequency. In fact, this is how the pulse channels
        // fundamentally work. A "duty cycle" shape is loaded into the
        // sequencer and the timer is used to vary the pitch, yielding
        // notes.

        struct sequencer
        {
            uint32_t sequence = 0x00000000;
            uint32_t new_sequence = 0x00000000;
            uint16_t timer = 0x0000;
            uint16_t reload = 0x0000;
            uint8_t output = 0x00;

            // Pass in a lambda function to manipulate the sequence as required
            // by the owner of this sequencer module
            uint8_t clock(bool bEnable, std::function<void(uint32_t &s)> funcManip);
        };

        struct lengthcounter
        {
            uint8_t counter = 0x00;
            uint8_t clock(bool bEnable, bool bHalt);
        };

        struct envelope
        {
            void clock(bool bLoop);

            bool start = false;
            bool disable = false;
            uint16_t divider_count = 0;
            uint16_t volume = 0;
            uint16_t output = 0;
            uint16_t decay_count = 0;
        };


        struct oscpulse
        {
            double frequency = 0;
            double dutycycle = 0;
            double amplitude = 1;
            double pi = 3.14159;
            double harmonics = 20;

            double sample(double t);
        };

        struct sweeper
        {
            bool enabled = false;
            bool down = false;
            bool reload = false;
            uint8_t shift = 0x00;
            uint8_t timer = 0x00;
            uint8_t period = 0x00;
            uint16_t change = 0;
            bool mute = false;

            void track(uint16_t &target);

            bool clock(uint16_t &target, bool channel);
        };

        double dGlobalTime = 0.0;

        // Square Wave Pulse Channel 1
        double pulse1_sample = 0.0;
        double pulse1_output = 0.0;
        sequencer pulse1_seq;
        oscpulse pulse1_osc;
        envelope pulse1_env;
        lengthcounter pulse1_lc;
        sweeper pulse1_sweep;

        // Square Wave Pulse Channel 2
        double pulse2_sample = 0.0;
        double pulse2_output = 0.0;
        sequencer pulse2_seq;
        oscpulse pulse2_osc;
        envelope pulse2_env;
        lengthcounter pulse2_lc;
        sweeper pulse2_sweep;

        // Noise Channel
        envelope noise_env;
        lengthcounter noise_lc;
        sequencer noise_seq;
        double noise_sample = 0;
        double noise_output = 0;

        uint16_t pulse1_visual = 0;
        uint16_t pulse2_visual = 0;
        uint16_t noise_visual = 0;
        uint16_t triangle_visual = 0;

    public:
        Core2A03();

        bool IsInit() const override { return m_audioOut.operator bool(); }
        void Reset() override;
        void Execute() override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

    private:
        double getOutputSample() const;
        void setSampleFrequency(uint32_t sample_rate);
    };

}
