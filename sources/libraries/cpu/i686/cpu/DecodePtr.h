//
// Created by pierr on 13/02/2024.
//
#pragma once

#include "EngineLib/data/Base.h"
#include "EngineLib/data/IDevice.h"

using namespace Astra::CPU;

class DecodePtr {
private:
    DWORD m_addr = 0;
    Astra::Ref<IDevice>& m_mem;

    const BYTE* m_prefetch = nullptr;

public:
    explicit DecodePtr(Astra::Ref<IDevice>& mem) : m_mem(mem) {}

    DWORD addr() const {return m_addr;}

    DecodePtr& operator=(DWORD addr) {
        m_addr = addr;
        m_prefetch = nullptr;
        return *this;
    }

    DecodePtr& operator=(const BYTE* prefetch) {
        m_prefetch = prefetch;
        m_addr = 0;
        return *this;
    }

    DecodePtr& operator+=(int v) {
        m_addr += v;
        return *this;
    }

    DecodePtr& operator++() {
        m_addr++;
        return *this;
    }

    DecodePtr operator++(int) {
        auto old = *this;
        m_addr++;
        return old;
    }

    DecodePtr& operator--() {
        m_addr--;
        return *this;
    }

    DecodePtr operator--(int) {
        auto old = *this;
        m_addr--;
        return old;
    }

    BYTE operator[](int addr) const {
        addr += m_addr;
        if (m_prefetch) {
            return m_prefetch[addr];
        } else {
            return m_mem->Fetch(DataFormat::Byte, addr);
        }
    }

    BYTE operator*() const {
        if (m_prefetch) {
            return m_prefetch[m_addr];
        } else {
            return m_mem->Fetch(DataFormat::Byte, m_addr);
        }
    }
};

