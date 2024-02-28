//
// Created by pierr on 18/10/2023.
//

#include "Core2A03.h"

namespace Astra::CPU::Lib {
    uint8_t Core2A03::length_table[] = {10, 254, 20, 2, 40, 4, 80, 6,
                                        160, 8, 60, 10, 14, 12, 26, 14,
                                        12, 16, 24, 18, 48, 20, 96, 22,
                                        192, 24, 72, 26, 16, 28, 32, 30};

    Core2A03::Core2A03() {
        Core2A03::Reset();
    }

    void Core2A03::Reset() {
        noise_seq.sequence = 0xDBDB;

        frame_clock_counter = 0;
        clock_counter = 0;

        dAudioSample = 0.0;
        dAudioTime = 0.0;
        dAudioTimePerNESClock = 0.0;
        dAudioTimePerSystemSample = 0.0f;

        setSampleFrequency(44100);
    }

    void Core2A03::Execute() {
        // Depending on the frame count, we set a flag to tell
        // us where we are in the sequence. Essentially, changes
        // to notes only occur at these intervals, meaning, in a
        // way, this is responsible for ensuring musical time is
        // maintained.
        bool bQuarterFrameClock = false;
        bool bHalfFrameClock = false;

        dGlobalTime += (0.3333333333 / 1789773);

        if (clock_counter % 6 == 0) {
            frame_clock_counter++;


            // 4-Step Sequence Mode
            if (frame_clock_counter == 3729) {
                bQuarterFrameClock = true;
            }

            if (frame_clock_counter == 7457) {
                bQuarterFrameClock = true;
                bHalfFrameClock = true;
            }

            if (frame_clock_counter == 11186) {
                bQuarterFrameClock = true;
            }

            if (frame_clock_counter == 14916) {
                bQuarterFrameClock = true;
                bHalfFrameClock = true;
                frame_clock_counter = 0;
            }

            // Update functional units

            // Quater frame "beats" adjust the volume envelope
            if (bQuarterFrameClock) {
                pulse1_env.clock(flags.pulse1_halt);
                pulse2_env.clock(flags.pulse2_halt);
                noise_env.clock(flags.noise_halt);
            }


            // Half frame "beats" adjust the note length and
            // frequency sweepers
            if (bHalfFrameClock) {
                pulse1_lc.clock(flags.pulse1_enable, flags.pulse1_halt);
                pulse2_lc.clock(flags.pulse2_enable, flags.pulse2_halt);
                noise_lc.clock(flags.noise_enable, flags.noise_halt);
                pulse1_sweep.clock(pulse1_seq.reload, 0);
                pulse2_sweep.clock(pulse2_seq.reload, 1);
            }

            //	if (bUseRawMode)
            {
                // Update Pulse1 Channel ================================
                pulse1_seq.clock(flags.pulse1_enable, [](uint32_t& s) {
                    // Shift right by 1 bit, wrapping around
                    s = ((s & 0x0001) << 7) | ((s & 0x00FE) >> 1);
                });

                //	pulse1_sample = (double)pulse1_seq.output;
            }
            //else
            {
                pulse1_osc.frequency = 1789773.0 / (16.0 * (double) (pulse1_seq.reload + 1));
                pulse1_osc.amplitude = (double) (pulse1_env.output - 1) / 16.0;
                pulse1_sample = pulse1_osc.sample(dGlobalTime);

                if (pulse1_lc.counter > 0 && pulse1_seq.timer >= 8 && !pulse1_sweep.mute && pulse1_env.output > 2)
                    pulse1_output += (pulse1_sample - pulse1_output) * 0.5;
                else
                    pulse1_output = 0;
            }

            //if (bUseRawMode)
            {
                // Update Pulse1 Channel ================================
                pulse2_seq.clock(flags.pulse2_enable, [](uint32_t& s) {
                    // Shift right by 1 bit, wrapping around
                    s = ((s & 0x0001) << 7) | ((s & 0x00FE) >> 1);
                });

                //	pulse2_sample = (double)pulse2_seq.output;

            }
            //	else
            {
                pulse2_osc.frequency = 1789773.0 / (16.0 * (double) (pulse2_seq.reload + 1));
                pulse2_osc.amplitude = (double) (pulse2_env.output - 1) / 16.0;
                pulse2_sample = pulse2_osc.sample(dGlobalTime);

                if (pulse2_lc.counter > 0 && pulse2_seq.timer >= 8 && !pulse2_sweep.mute && pulse2_env.output > 2)
                    pulse2_output += (pulse2_sample - pulse2_output) * 0.5;
                else
                    pulse2_output = 0;
            }


            noise_seq.clock(flags.noise_enable, [](uint32_t& s) {
                s = (((s & 0x0001) ^ ((s & 0x0002) >> 1)) << 14) | ((s & 0x7FFF) >> 1);
            });

            if (noise_lc.counter > 0 && noise_seq.timer >= 8) {
                noise_output = (double) noise_seq.output * ((double) (noise_env.output - 1) / 16.0);
            }

            if (!flags.pulse1_enable) pulse1_output = 0;
            if (!flags.pulse2_enable) pulse2_output = 0;
            if (!flags.noise_enable) noise_output = 0;

        }

        // Frequency sweepers change at high frequency
        pulse1_sweep.track(pulse1_seq.reload);
        pulse2_sweep.track(pulse2_seq.reload);

        pulse1_visual = (flags.pulse1_enable && pulse1_env.output > 1 && !pulse1_sweep.mute) ? pulse1_seq.reload : 2047;
        pulse2_visual = (flags.pulse2_enable && pulse2_env.output > 1 && !pulse2_sweep.mute) ? pulse2_seq.reload : 2047;
        noise_visual = (flags.noise_enable && noise_env.output > 1) ? noise_seq.reload : 2047;

        clock_counter++;

        flags.bAudioSampleReady = false;
        dAudioTime += dAudioTimePerNESClock;
        if (dAudioTime >= dAudioTimePerSystemSample) {
            dAudioTime -= dAudioTimePerSystemSample;
            dAudioSample = getOutputSample();
            flags.bAudioSampleReady = true;

            m_audioOut->Push(DataFormat::Large, 0, *(LARGE*) &dAudioSample);
        }
    }

    double Core2A03::getOutputSample() const {
        if (flags.bUseRawMode) {
            return (pulse1_sample - 0.5) * 0.5
                   + (pulse2_sample - 0.5) * 0.5;
        } else {
            return ((1.0 * pulse1_output) - 0.8) * 0.1 +
                   ((1.0 * pulse2_output) - 0.8) * 0.1 +
                   ((2.0 * (noise_output - 0.5))) * 0.1;
        }
    }

    void Core2A03::setSampleFrequency(uint32_t sample_rate) {
        dAudioTimePerSystemSample = 1.0 / (double) sample_rate;
        dAudioTimePerNESClock = 1.0 / 5369318.0; // PPU Clock Frequency
    }

    LARGE Core2A03::Fetch(DataFormat, size_t address) {
        uint8_t data = 0x00;

        if (address == 0x15) {
            //	data |= (pulse1_lc.counter > 0) ? 0x01 : 0x00;
            //	data |= (pulse2_lc.counter > 0) ? 0x02 : 0x00;
            //	data |= (noise_lc.counter > 0) ? 0x04 : 0x00;
        }

        return data;
    }

    void Core2A03::Push(DataFormat, size_t addr, LARGE data) {
        switch (addr) {
            case 0:
                switch ((data & 0xC0) >> 6) {
                    case 0x00:
                        pulse1_seq.new_sequence = 0b01000000;
                        pulse1_osc.dutycycle = 0.125;
                        break;
                    case 0x01:
                        pulse1_seq.new_sequence = 0b01100000;
                        pulse1_osc.dutycycle = 0.250;
                        break;
                    case 0x02:
                        pulse1_seq.new_sequence = 0b01111000;
                        pulse1_osc.dutycycle = 0.500;
                        break;
                    case 0x03:
                        pulse1_seq.new_sequence = 0b10011111;
                        pulse1_osc.dutycycle = 0.750;
                        break;
                }
                pulse1_seq.sequence = pulse1_seq.new_sequence;
                flags.pulse1_halt = (data & 0x20);
                pulse1_env.volume = (data & 0x0F);
                pulse1_env.disable = (data & 0x10);
                break;

            case 0x01:
                pulse1_sweep.enabled = data & 0x80;
                pulse1_sweep.period = (data & 0x70) >> 4;
                pulse1_sweep.down = data & 0x08;
                pulse1_sweep.shift = data & 0x07;
                pulse1_sweep.reload = true;
                break;

            case 0x02:
                pulse1_seq.reload = (pulse1_seq.reload & 0xFF00) | data;
                break;

            case 0x03:
                pulse1_seq.reload = (uint16_t) ((data & 0x07)) << 8 | (pulse1_seq.reload & 0x00FF);
                pulse1_seq.timer = pulse1_seq.reload;
                pulse1_seq.sequence = pulse1_seq.new_sequence;
                pulse1_lc.counter = length_table[(data & 0xF8) >> 3];
                pulse1_env.start = true;
                break;

            case 0x04:
                switch ((data & 0xC0) >> 6) {
                    case 0x00:
                        pulse2_seq.new_sequence = 0b01000000;
                        pulse2_osc.dutycycle = 0.125;
                        break;
                    case 0x01:
                        pulse2_seq.new_sequence = 0b01100000;
                        pulse2_osc.dutycycle = 0.250;
                        break;
                    case 0x02:
                        pulse2_seq.new_sequence = 0b01111000;
                        pulse2_osc.dutycycle = 0.500;
                        break;
                    case 0x03:
                        pulse2_seq.new_sequence = 0b10011111;
                        pulse2_osc.dutycycle = 0.750;
                        break;
                }
                pulse2_seq.sequence = pulse2_seq.new_sequence;
                flags.pulse2_halt = (data & 0x20);
                pulse2_env.volume = (data & 0x0F);
                pulse2_env.disable = (data & 0x10);
                break;

            case 0x05:
                pulse2_sweep.enabled = data & 0x80;
                pulse2_sweep.period = (data & 0x70) >> 4;
                pulse2_sweep.down = data & 0x08;
                pulse2_sweep.shift = data & 0x07;
                pulse2_sweep.reload = true;
                break;

            case 0x06:
                pulse2_seq.reload = (pulse2_seq.reload & 0xFF00) | data;
                break;

            case 0x07:
                pulse2_seq.reload = (uint16_t) ((data & 0x07)) << 8 | (pulse2_seq.reload & 0x00FF);
                pulse2_seq.timer = pulse2_seq.reload;
                pulse2_seq.sequence = pulse2_seq.new_sequence;
                pulse2_lc.counter = length_table[(data & 0xF8) >> 3];
                pulse2_env.start = true;

                break;

            case 0x08:
                break;

            case 0x0C:
                noise_env.volume = (data & 0x0F);
                noise_env.disable = (data & 0x10);
                flags.noise_halt = (data & 0x20);
                break;

            case 0x0E:
                switch (data & 0x0F) {
                    case 0x00:
                        noise_seq.reload = 0;
                        break;
                    case 0x01:
                        noise_seq.reload = 4;
                        break;
                    case 0x02:
                        noise_seq.reload = 8;
                        break;
                    case 0x03:
                        noise_seq.reload = 16;
                        break;
                    case 0x04:
                        noise_seq.reload = 32;
                        break;
                    case 0x05:
                        noise_seq.reload = 64;
                        break;
                    case 0x06:
                        noise_seq.reload = 96;
                        break;
                    case 0x07:
                        noise_seq.reload = 128;
                        break;
                    case 0x08:
                        noise_seq.reload = 160;
                        break;
                    case 0x09:
                        noise_seq.reload = 202;
                        break;
                    case 0x0A:
                        noise_seq.reload = 254;
                        break;
                    case 0x0B:
                        noise_seq.reload = 380;
                        break;
                    case 0x0C:
                        noise_seq.reload = 508;
                        break;
                    case 0x0D:
                        noise_seq.reload = 1016;
                        break;
                    case 0x0E:
                        noise_seq.reload = 2034;
                        break;
                    case 0x0F:
                        noise_seq.reload = 4068;
                        break;
                }
                break;

            case 0x0F:
                pulse1_env.start = true;
                pulse2_env.start = true;
                noise_env.start = true;
                noise_lc.counter = length_table[(data & 0xF8) >> 3];
                break;

            case 0x15: // APU STATUS
                flags.pulse1_enable = data & 0x01;
                flags.pulse2_enable = data & 0x02;
                flags.noise_enable = data & 0x04;
                break;
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{0x14, 0},
                                                                    {1,    0x15}};

    const std::vector<std::pair<size_t, size_t>>* Core2A03::GetDeviceAddressList() const {
        return &addrList;
    }

    ///////////////////////////////////////////////////////////

    void Core2A03::envelope::clock(bool bLoop) {
        if (!start) {
            if (divider_count == 0) {
                divider_count = volume;

                if (decay_count == 0) {
                    if (bLoop) {
                        decay_count = 15;
                    }

                } else
                    decay_count--;
            } else
                divider_count--;
        } else {
            start = false;
            decay_count = 15;
            divider_count = volume;
        }

        if (disable) {
            output = volume;
        } else {
            output = decay_count;
        }
    }

    double Core2A03::oscpulse::sample(double t) {
        double a = 0;
        double b = 0;
        double p = dutycycle * 2.0 * pi;

        auto approxsin = [](double t) {
            double j = t * 0.15915;
            j = j - (int) j;
            return 20.785 * j * (j - 0.5) * (j - 1.0);
        };

        for (double n = 1; n < harmonics; n++) {
            double c = n * frequency * 2.0 * pi * t;
            a += -approxsin(c) / n;
            b += -approxsin(c - p * n) / n;

            //a += -sin(c) / n;
            //b += -sin(c - p * n) / n;
        }

        return (2.0 * amplitude / pi) * (a - b);
    }

    void Core2A03::sweeper::track(uint16_t& target) {
        if (enabled) {
            change = target >> shift;
            mute = (target < 8) || (target > 0x7FF);
        }
    }

    bool Core2A03::sweeper::clock(uint16_t& target, bool channel) {
        bool changed = false;
        if (timer == 0 && enabled && shift > 0 && !mute) {
            if (target >= 8 && change < 0x07FF) {
                if (down) {
                    target -= change - channel;
                } else {
                    target += change;
                }
                changed = true;
            }
        }

        //if (enabled)
        {
            if (timer == 0 || reload) {
                timer = period;
                reload = false;
            } else
                timer--;

            mute = (target < 8) || (target > 0x7FF);
        }

        return changed;
    }

    uint8_t Core2A03::lengthcounter::clock(bool bEnable, bool bHalt) {
        if (!bEnable)
            counter = 0;
        else if (counter > 0 && !bHalt)
            counter--;
        return counter;
    }

    uint8_t Core2A03::sequencer::clock(bool bEnable, std::function<void(uint32_t&)> funcManip) {
        if (bEnable) {
            timer--;
            if (timer == 0xFFFF) {
                timer = reload;
                funcManip(sequence);
                output = sequence & 0x00000001;
            }
        }
        return output;
    }
}
