//
// Created by pierr on 19/10/2023.
//
#pragma once

#include "CpuEngine/engine/cpu/registers/IRegister.h"

namespace Astra::CPU::Core {

    template<typename T>
    class Register : public IRegister
    {
    private:
        T* m_data;
        const char* m_fmt;

    public:
        explicit Register(void* data, const char* fmt) : m_data((T*) data), m_fmt(fmt) {}

        LARGE GetValue() const override {
            return *m_data;
        }

        void SetValue(LARGE value) const override {
            *m_data = (T) value;
        }

        const char* GetFormat() const override {
            return m_fmt;
        }
    };

}
