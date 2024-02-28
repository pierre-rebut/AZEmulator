//
// Created by pierr on 13/02/2024.
//
#pragma once

#include "EngineLib/data/Base.h"
#include "EngineLib/data/IDevice.h"

using namespace Astra::CPU;

class ResultPtr {
private:
    DWORD m_addr = 0;
    Astra::Ref<IDevice>& m_mem;

    void* m_dataptr = nullptr;

public:
    explicit ResultPtr(Astra::Ref<IDevice>& mem) : m_mem(mem) {}
    ResultPtr(const ResultPtr& other, int addr) : m_addr(other.m_addr + addr), m_mem(other.m_mem), m_dataptr(other.m_dataptr) {}

    DWORD addr() const { return m_addr; }

    ResultPtr& operator=(DWORD addr) {
        m_addr = addr;
        m_dataptr = nullptr;
        return *this;
    }

    ResultPtr& operator=(void* dataptr) {
        m_dataptr = dataptr;
        m_addr = 0;
        return *this;
    }

    ResultPtr operator+(int val) {
        return {*this, val};
    }

    bool operator==(void* dataptr) const {
        if (m_dataptr) {
            return m_dataptr == dataptr;
        }

        return false;
    }

    template<typename T>
    T get(int idx = 0) const {
        if (m_dataptr) {
            return *(T*)((char*)m_dataptr + idx);
        } else {
            if constexpr (sizeof(T) == 1) {
                return m_mem->Fetch(DataFormat::Byte, m_addr + idx);
            } else if constexpr (sizeof(T) == 2) {
                return m_mem->Fetch(DataFormat::Word, m_addr + idx);
            } else if constexpr (sizeof(T) == 4) {
                return m_mem->Fetch(DataFormat::DWord, m_addr + idx);
            } else {
                return m_mem->Fetch(DataFormat::Large, m_addr + idx);
            }
        }
    }

    template<typename T>
    void set(T val, int idx = 0) {
        if (m_dataptr) {
            *((T*)(char*)m_dataptr + idx) = val;
        } else {
            if constexpr (sizeof(T) == 1) {
                m_mem->Push(DataFormat::Byte, m_addr + idx, val);
            } else if constexpr (sizeof(T) == 2) {
                m_mem->Push(DataFormat::Word, m_addr + idx, val);
            } else if constexpr (sizeof(T) == 4) {
                m_mem->Push(DataFormat::DWord, m_addr + idx, val);
            } else {
                m_mem->Push(DataFormat::Large, m_addr + idx, val);
            }
        }
    }
};

