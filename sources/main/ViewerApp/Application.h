//
// Created by pierr on 14/03/2023.
//
#pragma once

#include "EngineLib/data/Base.h"

namespace Astra {

    class Application
    {
    public:
        virtual ~Application() = default;
        virtual void run() = 0;
    };

}
