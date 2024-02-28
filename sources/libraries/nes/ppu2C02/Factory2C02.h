//
// Created by pierr on 15/10/2023.
//
#pragma once

#include "EngineLib/IFactory.h"

namespace Astra::CPU::Lib {

    class Factory2C02 : public IFactory
    {
    public:
        Factory2C02();
        Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const override;
    };

}
