//
// Created by pierr on 02/03/2022.
//

#pragma once

#include <cstdint>

namespace Astra {

    using UUID = uint64_t;

    class UUIDGen
    {
    public:
        UUIDGen() = delete;

        static UUID New();
    };
}
