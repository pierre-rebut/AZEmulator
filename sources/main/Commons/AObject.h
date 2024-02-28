//
// Created by pierr on 14/03/2023.
//
#pragma once

#include <string>
#include "EngineLib/data/Base.h"

namespace Astra {

    class AObject
    {
    public:
        virtual ~AObject() = default;
        virtual std::string toString() const;
    };

}

std::ostream& operator<<(std::ostream& out, const Astra::AObject& c);

std::ostream& operator<<(std::ostream& out, const Astra::AObject* c);