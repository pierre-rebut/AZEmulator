//
// Created by pierr on 11/03/2023.
//
#pragma once

#include <cstdint>

#include "Commons/Profiling.h"

#include "Commons/Log.h"
#include "EngineLib/data/IComObject.h"

namespace Astra::CPU::Core {

    class RamMemory : public IComObject
    {
    private:
        bool m_isReadOnly = false;
        size_t m_memSize{};
        BYTE* m_data;

    public:
        explicit RamMemory(size_t memSize);

        ~RamMemory() override;

        inline bool IsReadOnly() const {return m_isReadOnly;}
        void SetReadOnly(bool readOnly) {m_isReadOnly = readOnly;}

        void Clear();

        inline const BYTE* get() const { return m_data; }

        LARGE Fetch(DataFormat fmt, size_t address) override;
        void Push(DataFormat fmt, size_t address, LARGE value) override;

        inline size_t size() const {return m_memSize;}

        int FetchReadOnly(size_t address) const override;

        template<class CharT, class Traits>
        void dumpToStream(std::basic_ostream<CharT, Traits>& pOutStream) const {
            LOG_CPU_DEBUG("RamMemory: dump to stream");
            ENGINE_PROFILE_FUNCTION();

            pOutStream.write((char*) m_data, m_memSize);

            LOG_CPU_DEBUG("RamMemory: dump to stream end");
        }

        template<class CharT, class Traits>
        void loadFromStream(std::basic_istream<CharT, Traits>& pInStream, uint64_t startAddr = 0) const {
            LOG_CPU_DEBUG("RamMemory: load from stream");
            ENGINE_PROFILE_FUNCTION();

            pInStream.read((CharT*) m_data + startAddr, m_memSize - startAddr);

            LOG_CPU_DEBUG("RamMemory: load from stream end");
        }
    };

}
