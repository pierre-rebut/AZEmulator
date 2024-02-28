//
// Created by pierr on 19/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"
#include "EngineLib/services/IEntitiesService.h"

namespace Astra::CPU::Lib::CPU {

    class Factory : public IFactory
    {
    public:
        Factory();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}