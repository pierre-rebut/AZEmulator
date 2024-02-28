//
// Created by pierr on 05/09/2023.
//

#pragma once

namespace Astra::CPU::Core {

    template<typename T>
    class PointerInfo
    {
    private:
        T* m_data;
    public:
        explicit PointerInfo(T* data) : m_data(data) {}
        T GetValue() const {return *m_data;}
        void SetValue(const T& data) const {*m_data = data;}
    };

} // Astra
