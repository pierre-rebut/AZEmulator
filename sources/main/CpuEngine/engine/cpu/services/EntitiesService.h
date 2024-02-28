//
// Created by pierr on 25/08/2023.
//

#pragma once

#include <map>

#include "EngineLib/data/Base.h"
#include "EngineLib/services/IEntitiesService.h"

#include "CpuEngine/engine/cpu/registers/IRegister.h"
#include "CpuEngine/engine/cpu/registers/impl/PointerInfo.h"
#include "CpuEngine/engine/cpu/registers/impl/FlagRegister.h"
#include "CpuEngine/manager/buses/DataBus.h"

namespace Astra::CPU::Core {

    using DeviceValue = PointerInfo<Ref<IDevice>>;

    using RegistersInfo = std::map<std::string, Scope<IRegister>>;
    using FlagsInfo = std::map<std::string, Scope<IFlagRegister>>;
    using DevicesInfo = std::map<std::string, Scope<DeviceValue>>;

    struct ProgramCounterInfo
    {
        bool isValid = false;
        DeviceValue* dataBus;
        std::function<int()> pcFn;
        size_t registerSize;
    };

    class EntitiesService : public IEntitiesService
    {
    private:
        RegistersInfo m_registersInfo;
        RegistersInfo m_secondaryRegistersInfo;
        FlagsInfo m_flagsInfo;
        DevicesInfo m_devicesBinded;

        ProgramCounterInfo m_programCounterInfo{};

    public:
        void Reset();

        void AddNewRegister(const std::string& name, size_t size, void* data) override;
        void AddNewSecondaryRegister(const std::string& name, size_t size, void* data) override;
        void AddNewFlags(const std::string& name, void* data, size_t size, int index) override;
        void AddNewFlags(const std::string& name, bool* data) override;
        void BindDevice(const std::string& name, Ref<IDevice>* pBus) override;
        void SetProgramCounterInfo(const std::string& dataBusName, std::function<int()>&& pcFn, size_t registerSize) override;

        inline const RegistersInfo& GetRegisters() const {return m_registersInfo;}
        inline const RegistersInfo& GetSecondaryRegisters() const {return m_secondaryRegistersInfo;}
        inline const FlagsInfo& GetFlags() const {return m_flagsInfo;}
        inline const DevicesInfo& GetDevices() const {return m_devicesBinded;}
        inline const ProgramCounterInfo& GetProgramCounterInfo() const {return m_programCounterInfo;}

        void UnlinkDatabus(const Ref<DataBus>& databus) const;
    private:
        static Scope<IRegister> createRegisterFromType(size_t size, void* data);
    };

} // Astra
