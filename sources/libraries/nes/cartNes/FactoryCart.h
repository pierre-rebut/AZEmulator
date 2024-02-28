//
// Created by pierr on 20/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"
#include "EngineLib/services/IEntitiesService.h"

namespace Astra::CPU::Lib {

    class FactoryCart : public IFactory
    {
    public:
        FactoryCart();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}
