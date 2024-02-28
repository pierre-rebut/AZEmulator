//
// Created by pierr on 01/02/2024.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"
#include "EngineLib/data/KeyCodes.h"

#include <queue>

namespace Astra::CPU::Lib::Utils {

    class CoreKbr2Ascii : public ICpuCore
    {
    private:
        friend class Factory;

        bool m_onlyUpper = true;
        bool m_shift = false;
        bool m_ctrl = false;

        std::queue<char> m_charList;

        Ref<IDevice> m_keyboard = nullptr;

    public:
        bool IsInit() const override { return m_keyboard.operator bool(); };
        void Reset() override;
        void Execute() override;

        LARGE Fetch(DataFormat fmt, size_t address) override;
        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

        bool UpdateHardParameters(const std::vector<int>& hardParameters) override;

    private:
        void insertKeyToBuffer(KeyCode code);
        LARGE keyCodeToAscii(KeyCode code);

        inline bool isUpper() const {
            return m_shift || m_onlyUpper;
        }
    };

}
