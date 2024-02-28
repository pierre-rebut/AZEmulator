//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"
#include "EngineLib/services/IEntitiesService.h"

namespace Astra::CPU::Lib {

    class FactoryDMA : public IFactory
    {
    public:
        FactoryDMA();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}
