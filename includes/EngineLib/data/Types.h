//
// Created by pierr on 22/11/2022.
//
#pragma once

namespace Astra::CPU {
    using BYTE = unsigned char;
    using WORD = unsigned short;
    using DWORD = unsigned int;
    using LARGE = unsigned long long;

    enum class DataFormat {
        Byte = 0,
        Word = 1,
        DWord = 2,
        Large = 3
    };

    enum class CpuEngineType {
        STANDARD,
        DEVICE
    };

    enum class RunInfo {
        RUNNABLE,
        STATIC
    };

}
