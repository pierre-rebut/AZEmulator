//
// Created by pierr on 15/01/2024.
//
#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>

#include "EngineLib/data/Types.h"
#include "ymfm/src/ymfm.h"
#include "ymfm/src/ymfm_opm.h"

namespace Astra::CPU::Lib::X16 {
    class ym2151Interface : public ymfm::ymfm_interface
    {
    public:
        ym2151Interface() : m_chip(*this) {}

        void ymfm_sync_mode_write(BYTE data) override {
            m_engine->engine_mode_write(data);
        }

        void ymfm_sync_check_interrupts() override {
            m_engine->engine_check_interrupts();
        }

        void ymfm_set_timer(DWORD tnum, int32_t duration_in_clocks) override {
            if (tnum >= 2) return;
            m_timers[tnum] = duration_in_clocks;
        }

        void ymfm_set_busy_end(DWORD clocks) override {
            m_busy_timer = clocks;
        }

        bool ymfm_is_busy() override {
            return m_busy_timer > 0;
        }

        void ymfm_update_irq(bool asserted) override {
            m_irq_status = asserted;
        }

        void update_clocks(int cycles) {
            m_busy_timer = std::max(0, m_busy_timer - (64 * cycles));
            for (int i = 0; i < 2; ++i) {
                if (m_timers[i] > 0) {
                    m_timers[i] = std::max(0, m_timers[i] - (64 * cycles));
                    if (m_timers[i] <= 0) {
                        m_engine->engine_timer_expired(i);
                    }
                }
            }
        }

        void write(BYTE addr, BYTE value) {
            if (!ymfm_is_busy()) {
                m_chip.write_address(addr);
                m_chip.write_data(value);
            } else {
                std::cout << "YM2151 write received while busy." << std::endl;
            }
        }

        void generate(WORD* output, DWORD numsamples) {
            int s = 0;
            int ls, rs;
            update_clocks(numsamples);
            for (DWORD i = 0; i < numsamples; i++) {
                m_chip.generate(&opm_out);
                ls = opm_out.data[0];
                rs = opm_out.data[1];
                if (ls < -32768) ls = -32768;
                if (ls > 32767) ls = 32767;
                if (rs < -32768) rs = -32768;
                if (rs > 32767) rs = 32767;
                output[s++] = ls;
                output[s++] = rs;
            }
        }

        BYTE read_status() {
            return m_chip.read_status();
        }

        bool irq() {
            return m_irq_status;
        }

    private:
        ymfm::ym2151 m_chip;
        int32_t m_timers[2] = {0, 0};
        int32_t m_busy_timer = 0;
        bool m_irq_status = false;

        ymfm::ym2151::output_data opm_out;
    };
}
