//
// Created by pierr on 27/10/2023.
//
#include "Core8237.h"

namespace Astra::CPU::Lib::X86 {
    void Core8237::Reset() {
        m_flipflop = {};
        m_addr = {};
        m_cnt = {};
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{0x20, 0}};
    const std::vector<std::pair<size_t, size_t>>* Core8237::GetDeviceAddressList() const {
        return &addrList;
    }

    LARGE Core8237::Fetch(DataFormat, size_t address) {
        switch (address) {
            case 0x00:    // ADDR0
            case 0x02:    // ADDR1
            case 0x04:    // ADDR2
            case 0x06: {    // ADDR3
                size_t chan = address / 2;
                if (!m_flipflop[chan]) {
                    m_flipflop[chan] = true;
                    return m_addr[chan] & 0xff;
                } else {
                    m_flipflop[chan] = false;
                    return m_addr[chan] >> 8 & 0xff;
                }
            }
            case 0x01:    // CNT0
            case 0x03:    // CNT1
            case 0x05:    // CNT2
            case 0x07: {    // CNT3
                size_t chan = (address - 1) / 2;
                if (!m_flipflop[chan]) {
                    m_flipflop[chan] = true;
                    return m_cnt[chan] & 0xff;
                } else {
                    m_flipflop[chan] = false;
                    return m_cnt[chan] >> 8 & 0xff;
                }
            }

            default:
                return 0;
        }
    }

    void Core8237::Push(DataFormat, size_t address, LARGE value) {
        switch (address) {
            case 0x00:    // ADDR0
            case 0x02:    // ADDR1
            case 0x04:    // ADDR2
            case 0x06: {    // ADDR3
                size_t chan = address / 2;
                if (!m_flipflop[chan]) {
                    m_flipflop[chan] = true;
                    m_addr[chan] = m_addr[chan] & 0xff00 | value;
                } else {
                    m_flipflop[chan] = false;
                    m_addr[chan] = value << 8 | m_addr[chan] & 0xff;
                }
                break;
            }
            case 0x01:    // CNT0
            case 0x03:    // CNT1
            case 0x05:    // CNT2
            case 0x07: {    // CNT3
                size_t chan = (address - 1) / 2;
                if (!m_flipflop[chan]) {
                    m_flipflop[chan] = true;
                    m_cnt[chan] = m_cnt[chan] & 0xff00 | value;
                } else {
                    m_flipflop[chan] = false;
                    m_cnt[chan] = value << 8 | m_cnt[chan] & 0xff;
                }
                break;
            }
            default:
                break;
        }
    }
}
