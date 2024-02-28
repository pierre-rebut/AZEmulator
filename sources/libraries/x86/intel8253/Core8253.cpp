//
// Created by pierr on 27/10/2023.
//
#include "Core8253.h"

#include <iostream>

namespace Astra::CPU::Lib::X86 {
    void Core8253::Reset() {
        m_count = {};
        m_value = {};
        m_latch = {};
        m_control = {};
        m_enabled = {};
        m_latched = {};
        m_outputStatus = {};
        m_toggle = {};
    }

    void Core8253::Execute() {
        for (int sc = 0; sc < 3; ++sc) {
            if (!m_enabled[sc]) {
                continue;
            }

            switch (m_control[sc] >> 1 & 0b111) {
                case 0b00: {
                    m_count[sc] = --m_count[sc] & 0xffff;
                    if (m_count[sc] == 0) {
                        output(sc, true);
                    }
                    break;
                }
                case 0b10: {
                    m_count[sc] = --m_count[sc] & 0xffff;

                    if (m_count[sc] == 1) {
                        m_count[sc] = m_value[sc];
                        output(sc, false);
                    } else {
                        output(sc, true);
                    }
                    break;
                }
                case 0b11: {
                    if ((m_count[sc] & 0b1) == 0b1) {
                        if (m_outputStatus[sc]) {
                            m_count[sc] = m_count[sc] - 1 & 0xffff;
                        } else {
                            m_count[sc] = m_count[sc] - 3 & 0xffff;
                        }
                    } else {
                        m_count[sc] = m_count[sc] - 2 & 0xffff;
                    }

                    if (m_count[sc] == 0) {
                        m_count[sc] = m_value[sc];
                        output(sc, !m_outputStatus[sc]);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{4, 0x40}};

    const std::vector<std::pair<size_t, size_t>>* Core8253::GetDeviceAddressList() const {
        return &addrList;
    }

    LARGE Core8253::Fetch(DataFormat, size_t address) {

        int sc = address & 0b11;

        if (sc == 0b00 || sc == 0b01 || sc == 0b10) {
            int rl = m_control[sc] >> 4 & 0b11;
            int val = m_count[sc];
            if (m_latched[sc]) {
                val = m_latch[sc];
                if (rl < 0b11 || !m_toggle[sc]) {
                    m_latched[sc] = false;
                }
            }
            switch (rl) {
                case 0b01:    // Read least significant byte only.
                    return val & 0xff;
                case 0b10:    // Read most significant byte only.
                    return val >> 8 & 0xff;
                case 0b11: {   // Read lsb first, then msb.
                    if (!m_toggle[sc]) {
                        m_toggle[sc] = true;
                        return val & 0xff;
                    } else {
                        m_toggle[sc] = false;
                        return val >> 8 & 0xff;
                    }
                }
                default:
                    break;
            }
        }
        return 0;
    }

    void Core8253::Push(DataFormat, size_t address, LARGE val) {
        int sc = address & 0b11;
        switch (sc) {
            case 0b00:
            case 0b01:
            case 0b10: {
                int m = m_control[sc] >> 1 & 0b111;
                int rl = m_control[sc] >> 4 & 0b11;

                switch (rl) {
                    case 0b01:    // Load least significant byte only.
                        m_value[sc] = m_value[sc] & 0xff00 | val;
                        break;
                    case 0b10:    // Load most significant byte only.
                        m_value[sc] = val << 8 | m_value[sc] & 0xff;
                        break;
                    case 0b11: {   // Load lsb first, then msb.
                        if (!m_toggle[sc]) {
                            m_toggle[sc] = true;
                            m_value[sc] = m_value[sc] & 0xff00 | val;
                        } else {
                            m_toggle[sc] = false;
                            m_value[sc] = val << 8 | m_value[sc] & 0xff;
                        }
                        break;
                    }
                    default:
                        break;
                }
                if (rl < 0b11 || !m_toggle[sc]) {
                    m_count[sc] = m_value[sc];
                    m_enabled[sc] = true;
                    m_outputStatus[sc] = m == 0b10 || m == 0b11;
                }
                break;
            }
            case 0b11: {
                sc = val >> 6 & 0b11;

                if ((val >> 4 & 0b11) == 0b00) {
                    m_latch[sc] = m_count[sc];
                    m_latched[sc] = true;
                } else {
                    m_control[sc] = val & 0xffff;
                }
                break;
            }
            default:
                break;
        }
    }

    void Core8253::output(int sc, bool state) {
        if (!m_outputStatus[sc] && state) {
            if (sc == 0) { // TIMER 0
                //std::cout << "send int to pic : " << sc << " / " << state << std::endl;
                m_deviceService->SendDeviceInterrupt(0);
            }
        }
        m_outputStatus[sc] = state;
    }
}
