//
// Created by pierr on 18/10/2023.
//
#pragma once

#include "EngineLib/data/Types.h"

namespace Astra::CPU::Core {

    class IFlagRegister
    {
    protected:
        const int m_mask;

    public:
        explicit IFlagRegister(int index) : m_mask(1 << index) {}

        virtual ~IFlagRegister() = default;
        virtual bool GetValue() const = 0;
        virtual void SetValue(bool v) const = 0;
    };

    class BoolFlagRegister : public IFlagRegister
    {
    private:
        bool* m_data;

    public:
        explicit BoolFlagRegister(bool* data) : IFlagRegister(0), m_data(data) {}

        bool GetValue() const override { return *m_data; }

        void SetValue(bool v) const override { *m_data = v; }
    };

    template<typename T>
    class FlagRegister : public IFlagRegister
    {
    private:
        T* m_data;

    public:
        explicit FlagRegister(T* data, int index) : IFlagRegister(index), m_data(data) {}

        bool GetValue() const override { return ((*m_data) & m_mask) > 0; }

        void SetValue(bool v) const override {
            if (v)
                *m_data |= m_mask;
            else
                *m_data &= ~m_mask;
        }
    };

}
