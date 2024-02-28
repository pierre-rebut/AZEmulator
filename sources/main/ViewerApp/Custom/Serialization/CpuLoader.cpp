//
// Created by pierr on 25/07/2023.
//
#include "CpuLoader.h"

#include <fstream>

#include "Commons/Log.h"
#include "Commons/Profiling.h"
#include "Commons/AstraException.h"
#include "Commons/utils/YAMLimport.h"

#include "CpuEngine/manager/EngineManager.h"

namespace Astra::UI::App {
    bool CpuLoader::loadConfig(const std::filesystem::path& pFilename, std::vector<CPU::Core::CpuCreateData>& pData, UUID& lastCurrentEngine) noexcept {
        LOG_CPU_DEBUG("[CpuLoader] loadConfig {}", pFilename);
        ENGINE_PROFILE_FUNCTION();

        if (!std::filesystem::exists(pFilename)) {
            LOG_CPU_DEBUG("[CpuLoader] cpu config file not found, skipping.");
            return true;
        }

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["CPU"]) {
                throw AstraException("Invalid cpu config file: {}", pFilename);
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            LOG_CPU_TRACE("[CpuLoader] Starting loading");
            lastCurrentEngine = data["CurrentEngine"].as<UUID>(0);

            const auto cpus = data["CPU"];
            for (const auto cpu: cpus) {
                CPU::Core::CpuCreateData newCpu;

                const auto cpuUUID = cpu.first.as<UUID>();
                const auto& cpuInfo = cpu.second;

                LOG_CPU_TRACE("[CpuLoader]: loading cpu {}", cpuUUID);

                newCpu.uuid = cpuUUID;

                if (const auto dirInfo = cpuInfo["CoreLib"]) {
                    newCpu.coreLibDir = dirInfo["DirName"].as<std::string>("");
                    newCpu.coreLibName = dirInfo["LibName"].as<std::string>("");
                }
                newCpu.name = cpuInfo["Name"].as<std::string>("unknown");
                newCpu.autostart = cpuInfo["AutoStart"].as<bool>(true);
                newCpu.speed = cpuInfo["Speed"].as<size_t>(1000 * 1000);

                for (const auto param: cpuInfo["HardParameters"]) {
                    newCpu.hardParameters.emplace_back(param.as<int>(0));
                }

                newCpu.orderPriority = cpuInfo["OrderPriority"].as<int>(1);

                pData.emplace_back(std::move(newCpu));
            }
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("CpuLoader: load parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("CpuLoader: load error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("CpuLoader: loadConfig end");
        return true;
    }

    bool CpuLoader::saveConfig(const std::filesystem::path& pFilename, const AstraProject* project) noexcept {
        LOG_CPU_DEBUG("[CpuLoader]: saveValues {}", pFilename);

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "CurrentEngine" << YAML::Value << (project->getCurrentEngine() ? project->getCurrentEngine()->deviceUUID : 0);
            out << YAML::Key << "CPU" << YAML::Value << YAML::BeginMap;

            for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                out << YAML::Key << engineUUID << YAML::Value << YAML::BeginMap;

                out << YAML::Key << "Name" << YAML::Value << engine->GetName();

                out << YAML::Key << "CoreLib" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "DirName" << YAML::Value << engine->GetCpuCoreName().first;
                out << YAML::Key << "LibName" << YAML::Value << engine->GetCpuCoreName().second;
                out << YAML::EndMap;

                out << YAML::Key << "Speed" << YAML::Value << engine->GetRunService()->getCpuSpeed();
                out << YAML::Key << "AutoStart" << YAML::Value << engine->IsAutoStart();
                out << YAML::Key << "OrderPriority" << YAML::Value << engine->GetOrderPriority();

                out << YAML::Key << "HardParameters" << YAML::Value << YAML::BeginSeq;
                for (const auto& param: engine->GetHardParameters()) {
                    out << param;
                }
                out << YAML::EndSeq;

                out << YAML::EndMap;
            }

            out << YAML::EndMap;
            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("CpuLoader: save parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("CpuLoader: save error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("CpuLoader: saveValues end");
        return true;
    }
}
