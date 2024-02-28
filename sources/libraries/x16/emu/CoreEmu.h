//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/KeyCodes.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib::X16 {

    class CoreEmu : public ICpuCore
    {
    private:
        friend class Factory;

        bool debugger_enabled = false;
        bool disable_emu_cmd_keys = false;
        bool log_video = false;
        bool log_keyboard = false;
        BYTE echo_mode = 0;
        bool save_on_exit = false;

        LARGE clock_base = 0;
        LARGE clock_snap = 0;

    public:
        bool IsInit() const override { return true; }

        void Reset() override;

        void Push(DataFormat, size_t address, LARGE value) override;
        LARGE Fetch(DataFormat fmt, size_t address) override;
        int FetchReadOnly(size_t address) const override;

        const std::vector<std::pair<size_t, size_t>>* GetDeviceAddressList() const override;
    };

}
