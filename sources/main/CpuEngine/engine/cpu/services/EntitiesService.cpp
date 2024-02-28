//
// Created by pierr on 25/08/2023.
//

#include "EntitiesService.h"

#include "CpuEngine/engine/cpu/registers/impl/Register.h"

namespace Astra::CPU::Core {
    void EntitiesService::Reset() {
        m_registersInfo.clear();
        m_flagsInfo.clear();
        m_devicesBinded.clear();
        m_programCounterInfo = {false};
    }

    void EntitiesService::AddNewRegister(const std::string& name, size_t size, void* data) {
        m_registersInfo[name] = createRegisterFromType(size, data);
    }

    void EntitiesService::AddNewSecondaryRegister(const std::string& name, size_t size, void* data) {
        m_secondaryRegistersInfo[name] = createRegisterFromType(size, data);
    }

    Scope<IRegister> EntitiesService::createRegisterFromType(size_t size, void* data) {
        switch (size) {
            case sizeof(BYTE):
                return CreateScope<Register<BYTE>>(data, "%02X");
            case sizeof(WORD):
                return CreateScope<Register<WORD>>(data, "%04X");
            case sizeof(DWORD):
                return CreateScope<Register<DWORD>>(data, "%08X");
            default:
                return CreateScope<Register<LARGE>>(data, "%llX");
        }
    }

    void EntitiesService::AddNewFlags(const std::string& name, void* data, size_t size, int index) {
        switch (size) {
            case sizeof(BYTE): {
                m_flagsInfo[name] = CreateScope<FlagRegister<BYTE>>((BYTE*) data, index);
                break;
            }
            case sizeof(WORD): {
                m_flagsInfo[name] = CreateScope<FlagRegister<WORD>>((WORD*) data, index);
                break;
            }
            default: {
                m_flagsInfo[name] = CreateScope<FlagRegister<DWORD>>((DWORD*) data, index);
                break;
            }
        }
    }

    void EntitiesService::AddNewFlags(const std::string& name, bool* data) {
        m_flagsInfo[name] = CreateScope<BoolFlagRegister>(data);
    }

    void EntitiesService::BindDevice(const std::string& name, Ref<IDevice>* dev) {
        m_devicesBinded[name] = CreateScope<DeviceValue>(dev);
    }

    void EntitiesService::SetProgramCounterInfo(const std::string& dataBusName, std::function<int()>&& pcFn, size_t registerSize) {
        m_programCounterInfo.isValid = true;
        m_programCounterInfo.dataBus = m_devicesBinded.at(dataBusName).get();
        m_programCounterInfo.pcFn = std::move(pcFn);
        m_programCounterInfo.registerSize = registerSize;
    }

    void EntitiesService::UnlinkDatabus(const Ref<DataBus>& databus) const {
        for (const auto& [bindName, bindVal] : m_devicesBinded) {
            if (bindVal->GetValue() == databus) {
                bindVal->SetValue(nullptr);
            }
        }
    }
} // Astra