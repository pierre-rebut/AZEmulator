//
// Created by pierr on 27/08/2023.
//

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Commons/utils/UUID.h"

namespace Astra::UI::App {

    enum class PinMode {
        INPUT, OUTPUT
    };

    struct NodePin {
        int id;
        std::string name;
        PinMode mode;
    };

    struct NodeLink {
        int id;
        int pinIn;
        int pinOut;
    };

    struct NodeItem
    {
        UUID itemUUID;
        int nodeId;
        std::string name;
        std::vector<NodePin> pins;
        std::list<NodeLink> connections;

    };

} // astra
