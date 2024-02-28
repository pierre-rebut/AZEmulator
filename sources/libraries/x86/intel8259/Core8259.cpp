//
// Created by pierr on 27/10/2023.
//
#include "Core8259.h"

#include <iostream>

namespace Astra::CPU::Lib::X86 {
    void Core8259::Reset() {
        m_imr = 0;
        m_irr = 0;
        m_isr = 0;
        m_icwStep = 0;
        m_icw = {};
        forceInterrupt = false;
    }

    void Core8259::Execute() {
        int bits = m_irr & ~m_imr;
        if (bits > 0) {
            int interruptId = 0;

            for (int i = 0; i < 8; ++i) {
                if ((bits >> i & 0b1) > 0) {

                    m_irr ^= 1 << i;
                    m_isr |= 1 << i;
                    interruptId = m_icw[1] + i;
                    break;
                }
            }

            //std::cout << "send interrupt back to cpu : " << interruptId << std::endl;
            m_deviceService->SendDeviceInterrupt(interruptId);
        }

        if (forceInterrupt) {
            forceInterrupt = false;
            m_deviceService->SendDeviceNonMaskableInterrupt(8);
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{2, 0x20}};

    const std::vector<std::pair<size_t, size_t>>* Core8259::GetDeviceAddressList() const {
        return &addrList;
    }

    LARGE Core8259::Fetch(DataFormat, size_t address) {
        switch (address) {
            case 0x20:
                return m_irr;
            case 0x21:
                return m_imr;
            default:
                return 0;
        }
    }

    void Core8259::Push(DataFormat, size_t address, LARGE val) {
        switch (address) {
            case 0x20: {
                if ((val & 0x10) > 0) {
                    m_imr = 0;
                    m_icw[m_icwStep++] = val;
                }
                if ((val & 0x20) > 0)    // EOI
                {
                    for (int i = 0; i < 8; ++i) {
                        if ((m_isr >> i & 0b1) > 0) {
                            m_isr ^= 1 << i;
                        }
                    }
                }
                break;
            }
            case 0x21: {
                if (m_icwStep == 1) {
                    m_icw[m_icwStep++] = val;
                    if ((m_icw[0] & 0x02) > 0) {
                        ++m_icwStep;
                    }
                } else if (m_icwStep < 4) {
                    m_icw[m_icwStep++] = val;
                } else {
                    m_imr = val;
                }
                break;
            }
            default:
                break;
        }
    }

    void Core8259::Interrupt(bool isNmi, int interruptId) {
        m_irr |= 1 << interruptId;
    }
}
