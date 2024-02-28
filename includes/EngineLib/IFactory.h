//
// Created by pierr on 15/10/2023.
//
#pragma once

#include <memory>

#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/services/IEntitiesService.h"
#include "EngineLib/data/CoreConfigInfo.h"

namespace Astra::CPU {

    class IFactory
    {
    public:
        const bool isRunnable;

    protected:
        CoreConfigInfo coreConfig{};

    public:
        explicit IFactory(RunInfo runInfo) : isRunnable(runInfo == RunInfo::RUNNABLE) {}

        virtual ~IFactory() = default;

        virtual Scope<ICpuCore> CreateNewCore(IEntitiesService* entitiesService) const = 0;

        const CoreConfigInfo* GetCoreConfig() const {return &coreConfig;}
    };

}
