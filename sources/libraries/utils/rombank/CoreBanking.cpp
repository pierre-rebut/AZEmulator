//
// Created by pierr on 16/10/2023.
//

#include <cmath>
#include "CoreBanking.h"

namespace Astra::CPU::Lib::Utils {

    CoreBanking::CoreBanking() {
        m_addrList = {{1, 0}, {m_bankWindowSize, 1}};
    }

    void CoreBanking::Reset() {
        m_isInit = false;
        m_currentBank = 0;
        m_maxNbBank = 0;
        m_addrList = {{1, 0}, {m_bankWindowSize, 1}};

        if (m_romData && m_romData->Fetch(DataFormat::Byte, 0)) {
            m_romSize = m_romData->Fetch(DataFormat::Byte, 1);
            auto maxNbBank = std::ceil((double)m_romSize / m_bankWindowSize);
            if (maxNbBank >= 256) {
                m_maxNbBank = 0xFF;
            } else {
                m_maxNbBank = (BYTE) maxNbBank;
            }

            m_isInit = m_maxNbBank;
        }
    }

    LARGE CoreBanking::Fetch(DataFormat fmt, size_t address) {
        if (address == 0) {
            return m_currentBank;
        }

        address--;

        if (m_currentBank >= m_maxNbBank) {
            return ((address >> 8) + m_address) & 0xff;
        }

        return m_romData->Fetch(fmt, 2 + (m_currentBank * m_bankWindowSize) + address);
    }

    void CoreBanking::Push(DataFormat fmt, size_t address, LARGE value) {
        if (address == 0) {
            m_currentBank = (BYTE) value;
            return;
        }

        if (m_currentBank >= m_maxNbBank) {
            return ;
        }

        address--;
        m_romData->Push(fmt, 2 + (m_currentBank * m_bankWindowSize) + address, value);
    }

    int CoreBanking::FetchReadOnly(size_t address) const {
        if (address == 0) {
            return m_currentBank;
        }

        address--;

        if (m_currentBank >= m_maxNbBank) {
            return ((address >> 8) + m_address) & 0xff;
        }

        return m_romData->Fetch(DataFormat::Byte, 2 + (m_currentBank * m_bankWindowSize) + address);
    }

    bool CoreBanking::UpdateHardParameters(const std::vector<int>& hardParameters) {
        auto bankWindowSize = hardParameters.at(0);
        auto address = hardParameters.at(1);

        if (bankWindowSize <= 10 || bankWindowSize > 65536 || address < 0 || address > 256) {
            return false;
        }

        m_bankWindowSize = (WORD) bankWindowSize;
        m_address = (BYTE) address;

        CoreBanking::Reset();

        return true;
    }

    void CoreBanking::Interrupt(bool isNmi, int interruptId) {
        m_currentBank = 0;
    }

}
