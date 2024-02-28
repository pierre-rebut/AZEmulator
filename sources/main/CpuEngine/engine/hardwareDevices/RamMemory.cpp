//
// Created by pierr on 11/03/2023.
//
#include "RamMemory.h"
#include "Commons/format.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {
    RamMemory::RamMemory(std::size_t ramSize) : m_memSize(ramSize) {
        m_data = new BYTE[m_memSize];
    }

    RamMemory::~RamMemory() {
        LOG_CPU_DEBUG("[RamMemory] destroy");

        delete[] m_data;
    }

    void RamMemory::Clear() {
        for (std::size_t i = 0; i < m_memSize; i++) {
            m_data[i] = 0;
        }
    }

    LARGE RamMemory::Fetch(DataFormat fmt, size_t address) {
        if (address >= m_memSize) {
            return 0;
        }

        switch (fmt) {
            case DataFormat::Byte:
                return m_data[address];
            case DataFormat::Word:
                return *(WORD*) (m_data + address);
            case DataFormat::DWord:
                return *(DWORD*) (m_data + address);
            case DataFormat::Large:
                return *(LARGE*) (m_data + address);
            default:
                return 0;
        }
    }

    void RamMemory::Push(DataFormat fmt, size_t address, LARGE value) {
        if (address >= m_memSize || m_isReadOnly) {
            return;
        }

        switch (fmt) {
            case DataFormat::Byte:
                m_data[address] = value & 0xFF;
                break;
            case DataFormat::Word:
                *(WORD*) (m_data + address) = value & 0xFFFF;
                break;
            case DataFormat::DWord:
                *(DWORD*) (m_data + address) = value & 0xFFFFFFFF;
                break;
            case DataFormat::Large:
                *(LARGE*) (m_data + address) = value;
                break;
        }
    }

    int RamMemory::FetchReadOnly(size_t address) const {
        if (address < m_memSize) {
            return (int) m_data[address];
        }

        return 0;
    }
}
