//
// Created by pierr on 25/08/2023.
//

#pragma once

#include <cstddef>
#include <string>
#include <functional>

#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU {

    class IEntitiesService
    {
    public:
        virtual ~IEntitiesService() = default;

        virtual void AddNewRegister(const std::string& name, size_t size, void* data) = 0;
        virtual void AddNewSecondaryRegister(const std::string& name, size_t size, void* data) = 0;
        virtual void AddNewFlags(const std::string& name, void* data, size_t size, int index) = 0;
        virtual void AddNewFlags(const std::string& name, bool* data) = 0;
        virtual void BindDevice(const std::string& name, Ref<IDevice>*) = 0;
        virtual void SetProgramCounterInfo(const std::string& dataBusName, std::function<int()>&& pcFn, size_t registerSize) = 0;
    };

} // Astra
