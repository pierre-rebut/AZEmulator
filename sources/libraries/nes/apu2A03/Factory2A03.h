//
// Created by pierr on 18/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"

namespace Astra::CPU::Lib {

    class Factory2A03 : public IFactory
    {
    public:
        Factory2A03();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}
