//
// Created by pierr on 18/03/2023.
//
#include "EntitiesLoader.h"

#include <fstream>

#include "Commons/Log.h"
#include "Commons/Profiling.h"
#include "Commons/utils/YAMLimport.h"
#include "Commons/AstraException.h"

#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "CpuEngine/manager/buses/DataBusManager.h"
#include "Commons/utils/Singleton.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "ViewerApp/CoreLib/Events/NotificationEvent.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/CoreLib/System/Events.h"

namespace Astra::UI::App {

    bool EntitiesLoader::loadConfig(const Ref<CPU::Core::CpuEngine>& engine, const std::filesystem::path& pFilename) noexcept {
        LOG_CPU_DEBUG("EntitiesLoader: loadConfig {} for engine {}", pFilename, engine->deviceUUID);
        ENGINE_PROFILE_FUNCTION();

        if (!std::filesystem::exists(pFilename)) {
            LOG_CPU_DEBUG("EntitiesLoader: file not exists, skipped.");
            return true;
        }

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["RegistersValue"] || !data["FlagsValue"]) {
                throw AstraException("invalid config file: {}. ignored", pFilename);
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            if (data["IsCpuRunning"].as<bool>(false)) {
                engine->GetRunService()->Run();
            }

            const auto& registers = engine->GetEntities().GetRegisters();
            for (const auto reg: data["RegistersValue"]) {
                const auto regName = reg.first.as<std::string>();
                if (registers.contains(regName)) {
                    registers.at(regName)->SetValue(reg.second.as<unsigned int>(0));
                }
            }

            const auto& secondaryRegisters = engine->GetEntities().GetSecondaryRegisters();
            for (const auto reg: data["SecondaryRegistersValue"]) {
                const auto regName = reg.first.as<std::string>();
                if (secondaryRegisters.contains(regName)) {
                    secondaryRegisters.at(regName)->SetValue(reg.second.as<unsigned int>(0));
                }
            }

            const auto& flags = engine->GetEntities().GetFlags();
            for (const auto flag: data["FlagsValue"]) {
                const auto regName = flag.first.as<std::string>();
                if (flags.contains(regName)) {
                    flags.at(regName)->SetValue(flag.second.as<bool>(false));
                }
            }

            const auto& coreDataBuses = engine->GetEntities().GetDevices();
            const auto& dataBusManager = CPU::Core::DataBusManager::Get();
            const auto& devicesManager = CPU::Core::DevicesManager::Get();

            for (const auto bus: data["Devices"]) {
                const auto coreBusName = bus.first.as<std::string>();
                if (coreDataBuses.contains(coreBusName)) {
                    auto dataBusUUID = bus.second.as<UUID>(0);
                    Ref<CPU::IDevice> device = dataBusManager.GetDataBusByUUID(dataBusUUID);
                    if (device == nullptr) {
                        device = devicesManager.GetDeviceUUID(dataBusUUID);
                    }

                    coreDataBuses.at(coreBusName)->SetValue(device);
                }
            }

            for (const auto bus: data["Bus"]) {
                const auto coreBusName = bus.first.as<std::string>();
                if (coreDataBuses.contains(coreBusName)) {
                    auto dataBusUUID = bus.second.as<UUID>(0);
                    coreDataBuses.at(coreBusName)->SetValue(dataBusManager.GetDataBusByUUID(dataBusUUID));
                }
            }

            if (engine->IsCoreValid()) {
                if (!engine->UpdateHardParameters()) {
                    Core::Events::Get().OnEvent<Core::NotificationEvent>(
                            AstraMessage::New2(AstraMessageType::Warning, I18N::Get("PARAM_UPDATE_FAILED"), engine->deviceUUID));
                    return false;
                }

                if (CPU::Core::DataBusManager::Get().RefreshDeviceConnections(engine)) {
                    Core::Events::Get().OnEvent<Core::NotificationEvent>(
                            AstraMessage::New2(AstraMessageType::Warning, I18N::Get("REFRESH_CONNECTION_FAILED"), engine->deviceUUID));
                    return false;
                }
            }

        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("EntitiesLoader: load parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("EntitiesLoader: load error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("EntitiesLoader: loadConfig end");
        return true;
    }

    bool EntitiesLoader::saveValues(const Ref<CPU::Core::CpuEngine>& engine, const std::filesystem::path& pFilename) noexcept {
        LOG_CPU_DEBUG("EntitiesLoader: saveValues {} for engine {}", pFilename, engine->deviceUUID);
        ENGINE_PROFILE_FUNCTION();

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;

            out << YAML::Key << "IsCpuRunning" << YAML::Value << engine->GetRunService()->IsRunning();

            out << YAML::Key << "RegistersValue" << YAML::Value << YAML::BeginMap;
            for (const auto& [regName, reg]: engine->GetEntities().GetRegisters()) {
                out << YAML::Key << regName << YAML::Value << reg->GetValue();
            }
            out << YAML::EndMap;

            out << YAML::Key << "SecondaryRegistersValue" << YAML::Value << YAML::BeginMap;
            for (const auto& [regName, reg]: engine->GetEntities().GetSecondaryRegisters()) {
                out << YAML::Key << regName << YAML::Value << reg->GetValue();
            }
            out << YAML::EndMap;

            out << YAML::Key << "FlagsValue" << YAML::Value << YAML::BeginMap;
            for (const auto& [flagName, flagValue]: engine->GetEntities().GetFlags()) {
                out << YAML::Key << flagName << YAML::Value << flagValue->GetValue();
            }
            out << YAML::EndMap;

            out << YAML::Key << "Devices" << YAML::Value << YAML::BeginMap;
            for (const auto& [busName, busValue]: engine->GetEntities().GetDevices()) {
                auto val = busValue->GetValue();
                auto valueUUID = val ? val->deviceUUID : 0;
                out << YAML::Key << busName << YAML::Value << valueUUID;
            }
            out << YAML::EndMap;

            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("EntitiesLoader: save parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("EntitiesLoader: save error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("EntitiesLoader: saveValues end");
        return true;
    }
}
