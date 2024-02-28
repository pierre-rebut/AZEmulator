//
// Created by pierr on 22/08/2023.
//

#pragma once

#include "Commons/AstraException.h"

namespace Astra::CPU::Core {

    class CppException : public AstraException
    {
    public:
        template<typename... Args>
        explicit CppException(const std::string_view& pMsg, Args&&... pArgs) : AstraException(pMsg, std::forward<Args>(pArgs)...) {}

        const char* name() const override { return "CppException"; }

        template<typename... Args>
        inline static void assertV(bool test, const std::string_view& fmt, Args&&... args) {
            AstraException::internalAssertV<CppException>(test, fmt, std::forward<Args>(args)...);
        }
    };

} // Astra
