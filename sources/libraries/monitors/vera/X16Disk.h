//
// Created by pierr on 19/01/2024.
//
#pragma once

#include <cstddef>

#include "EngineLib/data/Types.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib {

    enum class AstraDiskSeekDir {
        XSEEK_SET,
        XSEEK_END,
        XSEEK_CUR
    };

    class X16Disk
    {
    private:
        Ref<IDevice>& m_diskDevice;

        bool m_isReady = false;
        size_t m_diskSize = 0;
        int64_t m_diskPos = 0;

    public:
        explicit X16Disk(Ref<IDevice>& diskDevice) : m_diskDevice(diskDevice) {}

        void reset();

        bool isAttached() const {return m_isReady;}
        size_t size() const {return m_diskSize;}

        size_t read(BYTE *data, size_t data_size, size_t data_count);
        size_t write(const BYTE *data, size_t data_size, size_t data_count);
        int seek(size_t pos, AstraDiskSeekDir origin);
    };

}
