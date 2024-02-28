//
// Created by pierr on 19/01/2024.
//
#pragma once

#include <array>

#include "EngineLib/data/Types.h"
#include "X16Disk.h"

namespace Astra::CPU::Lib::Monitors {

    class SdCard
    {
    private:
        BYTE rxbuf[3 + 512];
        int rxbuf_idx = 0;
        DWORD lba = 0;
        BYTE last_cmd = 0;
        bool is_acmd = false;
        bool is_idle = true;
        bool is_initialized = false;

        const BYTE *response = nullptr;
        int response_length = 0;
        int response_counter = 0;

        bool selected = false;

        std::array<BYTE, 21> rr{};

        X16Disk& sdCardFile;

    public:
        explicit SdCard(X16Disk& sd) : sdCardFile(sd) {}

        void reset();
        bool isAttached() const {return sdCardFile.isAttached();}

        void select(bool select);
        BYTE handle(BYTE inbyte);

    private:
        void set_response_csd();
        void set_response_r1();
        void set_response_r2();
        void set_response_r3();
        void set_response_r7();
    };

}
