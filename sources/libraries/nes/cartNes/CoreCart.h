//
// Created by pierr on 20/10/2023.
//
#pragma once

#include <memory>
#include <vector>

#include "EngineLib/core/ICpuCore.h"

#include "IMapper.h"
#include "EngineLib/data/Base.h"
#include "EngineLib/data/IDevice.h"

namespace Astra::CPU::Lib {

    class CoreCart : public ICpuCore
    {
    private:
        friend class FactoryCart;

        union Flags
        {
            struct
            {
                bool isImageReady : 1 = false;
                BYTE unused : 7;
            };

            BYTE reg;
        } flags;

        size_t m_cartCurrentAddress = 2;
        MIRROR hw_mirror = MIRROR::HORIZONTAL;

        BYTE nMapperID = 0;
        BYTE nPRGBanks = 0;
        BYTE nCHRBanks = 0;

        std::vector<BYTE> vPRGMemory;
        std::vector<BYTE> vCHRMemory;

        Ref<IMapper> m_mapper = nullptr;

        Ref<IDevice> m_cartSlot = nullptr;

    public:
        CoreCart();

        bool IsInit() const override { return m_mapper && m_cartSlot && flags.isImageReady; }
        void Reset() override;
        void Execute() override;

        LARGE Fetch(DataFormat fmt, size_t address) override;
        void Push(DataFormat fmt, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

        int FetchReadOnly(size_t address) const override;

    private:
        BYTE getByte(size_t address);
        void setByte(size_t address, BYTE value);

        bool cartSlotRead();

        void readFromDisk(char* buffer, int bufferSize);
    };

}
