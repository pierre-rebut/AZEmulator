//
// Created by pierr on 19/01/2024.
//
#pragma once

#include <cstddef>
#include "Types.h"

namespace Astra::CPU {

    class IComObject {
    public:
        virtual ~IComObject() = default;

        virtual LARGE Fetch(DataFormat fmt, size_t address) {return 0;}

        virtual void Push(DataFormat fmt, size_t address, LARGE value) { /* do nothing */ }

        virtual int FetchReadOnly(size_t address) const { return -1; }
    };
}
