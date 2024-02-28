//
// Created by pierr on 22/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"
#include "EngineLib/services/IEntitiesService.h"

namespace Astra::CPU::Lib::CPU8086 {

    class Factory : public IFactory
    {
    public:
        Factory();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}
