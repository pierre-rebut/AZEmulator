//
// Created by pierr on 02/03/2022.
//


#include "UUID.h"

#include "Commons/utils/RandomGenerator.h"

namespace Astra {

    UUID UUIDGen::New() {
        return RandomGenerator::Get()->rndInt();
    }
}
