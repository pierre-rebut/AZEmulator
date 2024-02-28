//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

#include <unordered_map>

namespace Astra::CPU::Lib::X16 {

    class CoreBanking : public ICpuCore
    {
    private:
        friend class Factory;

        BYTE m_address = 0;
        WORD m_bankWindowSize = 8192;

        bool m_isInit = false;
        BYTE m_maxNbBank = 0;
        BYTE m_currentBank = 0;

        size_t m_romSize = 0;
        Ref<IDevice> m_romData = nullptr;

        std::vector<std::pair<size_t, size_t>> m_addrList;

    public:
        CoreBanking();

        bool IsInit() const override { return m_isInit; }

        void Reset() override;
        void Interrupt(bool isNmi, int interruptId) override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;
        int FetchReadOnly(size_t address) const override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override { return &m_addrList; }

        bool UpdateHardParameters(const std::vector<int>& hardParameters) override;
    };

}
