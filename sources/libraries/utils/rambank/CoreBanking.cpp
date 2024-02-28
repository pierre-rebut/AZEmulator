//
// Created by pierr on 16/10/2023.
//

#include "CoreBanking.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::Utils {

    CoreBanking::CoreBanking() {
        m_addrList = {{1,                0},
                      {m_bankWindowSize, 1}};
    }

    CoreBanking::~CoreBanking() {
        deleteAllBank();
    }

#define RDNUM 42

    void CoreBanking::Reset() {
        m_currentBank = 0;
        m_addrList = {{1,                0},
                      {m_bankWindowSize, 1}};
        deleteAllBank();

        for (int bankId = 0; bankId <= m_maxBankId; bankId++) {
            auto newBank = new BYTE[m_bankWindowSize];
            for (int pos = 0; pos < m_bankWindowSize; pos++) {
                newBank[pos] = ((bankId + 1) * RDNUM + ((pos + 1) * RDNUM + 1)) * RDNUM;
            }
            m_bankData.emplace_back(newBank);
        }
    }

    void CoreBanking::deleteAllBank() {
        for (const auto& bank: m_bankData) {
            delete[] bank;
        }

        m_bankData.clear();
    }

    LARGE CoreBanking::Fetch(DataFormat fmt, size_t address) {
        if (address == 0) {
            return m_currentBank;
        }

        address--;

        if (m_currentBank > m_maxBankId) {
            return ((address >> 8) + m_address) & 0xff;
        }

        LARGE ret = m_bankData.at(m_currentBank)[address];
        if (fmt == DataFormat::Word) {
            ret |= (LARGE) (m_bankData.at(m_currentBank)[address + 1]) << 8;
        }
        return ret;
    }

    void CoreBanking::Push(DataFormat fmt, size_t address, LARGE value) {
        if (address == 0) {
            m_currentBank = (BYTE) value;
            return;
        }

        if (m_currentBank > m_maxBankId) {
            return;
        }

        address--;
        m_bankData.at(m_currentBank)[address] = (BYTE) value;

        if (fmt == DataFormat::Word) {
            m_bankData.at(m_currentBank)[address + 1] = (BYTE) (value >> 8);
        }
    }

    int CoreBanking::FetchReadOnly(size_t address) const {
        if (address == 0) {
            return m_currentBank;
        }

        address--;
        if (m_currentBank > m_maxBankId) {
            return ((address >> 8) + m_address) & 0xff;
        }

        return m_bankData.at(m_currentBank)[address];
    }

    bool CoreBanking::UpdateHardParameters(const std::vector<int>& hardParameters) {
        auto bankWindowSize = hardParameters.at(0);
        auto maxBankId = hardParameters.at(1);
        auto address = hardParameters.at(2);

        if (bankWindowSize <= 10 || bankWindowSize > 65536 || maxBankId <= 0 || maxBankId >= 256 || address < 0 || address > 256) {
            return false;
        }

        m_bankWindowSize = (WORD) bankWindowSize;
        m_maxBankId = (BYTE) maxBankId;
        m_address = (BYTE) address;

        CoreBanking::Reset();

        return true;
    }

}
