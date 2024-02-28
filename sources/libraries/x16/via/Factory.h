//
// Created by pierr on 16/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"
#include "EngineLib/services/IEntitiesService.h"

namespace Astra::CPU::Lib::X16 {

    class Factory : public IFactory
    {
    public:
        Factory();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}
