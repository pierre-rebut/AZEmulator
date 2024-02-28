//
// Created by pierr on 27/10/2023.
//
#pragma once

#include <array>

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::X86 {

    class Core8255 : public ICpuCore
    {
    private:
        friend class Factory;

        union {
            struct {
                bool OutputBufferFull : 1;
                bool InputBufferFull : 1;
                bool SystemFlag : 1;
                bool CommandData : 1;
                bool KeyboardInhibit : 1;
                bool AuxiliaryDeviceOutputBuffer : 1;
                bool GeneralPurposeTimeOut : 1;
                bool ParityError : 1;
            };

            BYTE reg = 0;
        } status;

        std::array<int, 3> ports;

        Ref<IDevice> m_keyboard;

    public:

        bool IsInit() const override {return m_keyboard.operator bool();}

        void Execute() override;
        void Reset() override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;

        LARGE Fetch(DataFormat, size_t address) override;
        void Push(DataFormat, size_t address, LARGE value) override;

    private:
        int keyCodeTox86KeyCode(KeyCode keyCode);
    };

}
