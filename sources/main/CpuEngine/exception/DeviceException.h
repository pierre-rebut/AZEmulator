//
// Created by pierr on 22/08/2023.
//

#pragma once

#include "Commons/AstraException.h"

namespace Astra::CPU::Core {

    class DeviceException : public AstraException
    {
    public:
        template<typename... Args>
        explicit DeviceException(const std::string_view& pMsg, Args&&... pArgs) : AstraException(pMsg, std::forward<Args>(pArgs)...) {}

        const char* name() const override { return "DeviceException"; }
    };

} // Astra
