//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"
#include "Via.h"
#include "I2C.h"
#include "Serial.h"

#include <unordered_map>

namespace Astra::CPU::Lib::X16 {

    class CoreVia : public ICpuCore
    {
    private:
        friend class Factory;

        bool set_system_time = false;
        bool sendNmi = false;

        Ref<IDevice> m_keyboard;
        Ref<IDevice> m_mouse;

        Smc smc{sendNmi, m_keyboard, m_mouse};
        Rtc rtc;
        Serial serial;

       Via via[2]{};
       I2C i2c{smc, rtc};

    public:
        CoreVia();

        bool IsInit() const override { return m_keyboard && m_mouse; }

        void Reset() override;
        void Execute() override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

        bool UpdateHardParameters(const std::vector<int>& hardParameters) override;

    private:
        BYTE readVia1(BYTE reg, bool debug);
        void writeVia1(BYTE reg, BYTE value);

        BYTE readVia2(BYTE reg, bool debug);
        void writeVia2(BYTE reg, BYTE value);
    };

}
