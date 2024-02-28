//
// Created by pierr on 11/02/2024.
//
#pragma once

#include "Base.h"
#include "IDevice.h"

namespace Astra::CPU {

    template<typename T>
    class DevicePtr
    {
    private:
        const Ref<IDevice> m_basePtr;
        size_t m_addrOffset;

    public:
        explicit DevicePtr(const Ref<IDevice>& basePtr, size_t addrOffset = 0) : m_basePtr(basePtr), m_addrOffset(addrOffset) {}
        DevicePtr(const DevicePtr& devicePtr, size_t addrOffset = 0) : m_basePtr(devicePtr.m_basePtr), m_addrOffset(devicePtr.m_addrOffset + (addrOffset * sizeof(T))) {}
        DevicePtr& operator=(const DevicePtr& other) = default;
        DevicePtr& operator=(DevicePtr&& other) = default;

        LARGE Fetch(DataFormat fmt, size_t address) const {
            return m_basePtr->Fetch(fmt, address + m_addrOffset);
        }

        void Push(DataFormat fmt, size_t address, LARGE value) const {
            m_basePtr->Push(fmt, address + m_addrOffset, value);
        }

        T operator[](int addr) const {
            if constexpr (sizeof(T) == 1) {
                return Fetch(DataFormat::Byte, addr + m_addrOffset);
            } else if constexpr (sizeof(T) == 2) {
                return Fetch(DataFormat::Word, addr + m_addrOffset);
            } else if constexpr (sizeof(T) == 4) {
                return Fetch(DataFormat::DWord, addr + m_addrOffset);
            } else {
                return Fetch(DataFormat::Large, addr + m_addrOffset);
            }
        }

        DevicePtr operator+(int addr) const {
            return {*this, addr};
        }

        DevicePtr& operator+=(int addr) {
            m_addrOffset += addr;
            return *this;
        }

        DevicePtr& operator++() {
            m_addrOffset += sizeof(T);
            return *this;
        }

        DevicePtr& operator--() {
            m_addrOffset -= sizeof(T);
            return *this;
        }
    };

}
