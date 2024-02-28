//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

#include <vector>

namespace Astra::CPU::Lib::Utils {

    class CoreBanking : public ICpuCore
    {
    private:
        friend class Factory;

        WORD m_bankWindowSize = 8192;
        BYTE m_maxBankId = 32;
        BYTE m_address = 0;

        BYTE m_currentBank = 0;
        std::vector<BYTE*> m_bankData;

        std::vector<std::pair<size_t, size_t>> m_addrList;

    public:
        CoreBanking();
        ~CoreBanking() override;

        bool IsInit() const override { return m_bankWindowSize > 0; }
        void Reset() override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;
        int FetchReadOnly(size_t address) const override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override { return &m_addrList; }

        bool UpdateHardParameters(const std::vector<int>& hardParameters) override;

    private:
        void deleteAllBank();
    };

}
