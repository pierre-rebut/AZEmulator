//
// Created by pierr on 24/08/2023.
//

#pragma once

namespace Astra::CPU::Core {

    class IRegister {
    public:
        virtual ~IRegister() = default;

        virtual LARGE GetValue() const = 0;
        virtual void SetValue(LARGE) const = 0;
        virtual const char* GetFormat() const = 0;
    };

} // Astra
