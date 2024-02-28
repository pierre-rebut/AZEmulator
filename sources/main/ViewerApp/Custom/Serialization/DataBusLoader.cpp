//
// Created by pierr on 26/08/2023.
//

#include "DataBusLoader.h"

#include "Commons/Profiling.h"
#include "Commons/Log.h"
#include "Commons/utils/YAMLimport.h"

#include "CpuEngine/manager/buses/DataBusManager.h"

namespace Astra::UI::App {
    bool DataBusLoader::loadConfig(const std::filesystem::path& filename, std::vector<CPU::Core::BusCreateData>& dataResult) noexcept {
        LOG_CPU_DEBUG("[DataBusLoader] loadConfig {}", filename);
        ENGINE_PROFILE_FUNCTION();

        if (!std::filesystem::exists(filename)) {
            LOG_CPU_DEBUG("[DataBusLoader] bus config file not found, skipping.");
            return true;
        }

        try {
            YAML::Node data = YAML::LoadFile(filename.string());
            if (!data["Version"] || !data["Bus"]) {
                throw AstraException("Invalid bus config file: {}", filename);
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", filename);
            }

            LOG_CPU_TRACE("[DataBusLoader] Starting loading");

            for (const auto bus: data["Bus"]) {
                CPU::Core::BusCreateData newBus;

                const auto busUUID = bus.first.as<UUID>();
                const auto& busInfo = bus.second;

                LOG_CPU_TRACE("[DataBusLoader]: loading bus {}", busUUID);

                newBus.uuid = busUUID;
                newBus.name = busInfo["Name"].as<std::string>("unknown");
                newBus.size = busInfo["Size"].as<size_t>(0);
                newBus.isReadOnly = busInfo["ReadOnly"].as<bool>(false);

                for (const auto deviceInfo: busInfo["ConnectedDevices"]) {
                    newBus.connectedDevices.emplace_back(
                            deviceInfo["DeviceUUID"].as<size_t>(),
                            deviceInfo["Address"].as<size_t>(0),
                            deviceInfo["Index"].as<size_t>(0)
                    );
                }

                dataResult.emplace_back(std::move(newBus));
            }
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("DataBusLoader: load parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("DataBusLoader: load error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("DataBusLoader: loadConfig end");
        return true;
    }

    bool DataBusLoader::saveConfig(const std::filesystem::path& filename) noexcept {
        LOG_CPU_DEBUG("[DataBusLoader]: saveValues {}", filename);

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "Bus" << YAML::Value << YAML::BeginMap;

            for (const auto& [busUUID, bus]: CPU::Core::DataBusManager::Get().GetDataBuses()) {
                out << YAML::Key << busUUID << YAML::Value << YAML::BeginMap;

                out << YAML::Key << "Name" << YAML::Value << bus->GetName();
                out << YAML::Key << "Size" << YAML::Value << bus->GetBusSize();
                out << YAML::Key << "ReadOnly" << YAML::Value << bus->IsReadOnly();

                out << YAML::Key << "ConnectedDevices" << YAML::Value << YAML::BeginSeq;
                for (const auto& connectedDevice : bus->GetLinkedDevices()) {
                    out << YAML::BeginMap;
                    out << YAML::Key << "DeviceUUID" << YAML::Value << connectedDevice.device->deviceUUID;
                    out << YAML::Key << "Address" << YAML::Value << connectedDevice.addressLow;
                    out << YAML::Key << "Index" << YAML::Value << connectedDevice.index;
                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;

                out << YAML::EndMap;
            }

            out << YAML::EndMap;
            out << YAML::EndMap;

            std::ofstream fileOut(filename);
            AstraException::assertV(fileOut.good(), "can not open file {}", filename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("DataBusLoader: save parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("DataBusLoader: save error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("DataBusLoader: saveValues end");
        return true;
    }
} // Astra