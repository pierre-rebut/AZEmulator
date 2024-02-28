//
// Created by pierr on 14/03/2023.
//
#include "AObject.h"
#include "format.h"

namespace Astra {
    std::string AObject::toString() const {
        return myFormat("Object<UNDEFINED>");
    }
}

std::ostream& operator<<(std::ostream& out, const Astra::AObject& c) {
    out << c.toString();
    return out;
}

std::ostream& operator<<(std::ostream& out, const Astra::AObject* c) {
    out << c->toString();
    return out;
}
