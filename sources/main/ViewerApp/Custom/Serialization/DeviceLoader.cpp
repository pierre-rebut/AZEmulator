//
// Created by pierr on 15/08/2023.
//
#include "DeviceLoader.h"

#include "Commons/Log.h"
#include "Commons/Profiling.h"
#include "Commons/utils/YAMLimport.h"

#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "CpuEngine/engine/hardwareDevices/impl/ScreenDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/AudioDevice.h"
#include "CpuEngine/engine/hardwareDevices/impl/DiskDevice.h"

namespace Astra::UI::App {
    bool DeviceLoader::loadConfig(const std::filesystem::path& pFilename, std::vector<CPU::Core::DeviceCreateData>& pData) noexcept {
        LOG_CPU_DEBUG("[DeviceLoader] loadConfig {}", pFilename);
        ENGINE_PROFILE_FUNCTION();

        if (!std::filesystem::exists(pFilename)) {
            LOG_CPU_DEBUG("[DeviceLoader] devices config file not found, skipping.");
            return true;
        }

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["Devices"]) {
                throw AstraException("invalid devices config file: {}", pFilename);
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            LOG_CPU_TRACE("[DeviceLoader] Starting loading");

            const auto devices = data["Devices"];
            for (const auto device: devices) {
                CPU::Core::DeviceCreateData newDev;

                const auto deviceUUID = device.first.as<UUID>();
                const auto& deviceInfo = device.second;

                LOG_CPU_TRACE("[DeviceLoader] loading device {}", deviceUUID);

                newDev.uuid = deviceUUID;
                newDev.type = CPU::Core::Device::convertDeviceTypeFromString(deviceInfo["Type"].as<std::string>("unknown"));
                newDev.name = deviceInfo["Name"].as<std::string>("undefined");

                for (const auto cpu: deviceInfo["ConnectedCpu"]) {
                    newDev.connectedCpu.emplace_back(cpu.as<UUID>(0));
                }

                if (const auto screenInfo = deviceInfo["Screen"]) {
                    newDev.screenData.width = screenInfo["Width"].as<size_t>(256);
                    newDev.screenData.height = screenInfo["Height"].as<size_t>(240);
                } else if (const auto diskInfo = deviceInfo["Disk"]) {
                    auto diskId = diskInfo["DiskId"].as<UUID>(0);
                    const auto& metadata = UI::Core::AssetManager::Get().GetMetadata(diskId);
                    if (metadata.isValid()) {
                        newDev.diskData.diskId = diskId;
                        newDev.diskData.diskPath = UI::Core::AssetMetadata::GetFileSystemPath(metadata);
                    }
                    newDev.diskData.readOnly = diskInfo["ReadOnly"].as<bool>(false);
                } else if (const auto audioInfo = deviceInfo["MasterVolume"]) {
                    newDev.audioData.masterVolume = audioInfo.as<float>(1);
                }

                pData.emplace_back(std::move(newDev));
            }
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("DeviceLoader: load parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("DeviceLoader: load error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("DeviceLoader: loadConfig end");
        return true;
    }

    bool DeviceLoader::saveConfig(const std::filesystem::path& pFilename) noexcept {
        LOG_CPU_DEBUG("[DeviceLoader]: saveValues {}", pFilename);

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "Devices" << YAML::Value << YAML::BeginMap;

            for (const auto& [deviceUUID, device]: CPU::Core::DevicesManager::Get().GetDevices()) {
                out << YAML::Key << deviceUUID << YAML::Value << YAML::BeginMap;

                out << YAML::Key << "Name" << YAML::Value << device->GetName();
                out << YAML::Key << "Type" << YAML::Value << CPU::Core::Device::convertDeviceTypeToString(device->type);

                switch (device->type) {
                    using enum CPU::Core::DeviceType;
                    case SCREEN: {
                        const auto& screenDevice = std::dynamic_pointer_cast<CPU::Core::ScreenDevice>(device);
                        out << YAML::Key << "Screen" << YAML::Value << YAML::BeginMap;
                        out << YAML::Key << "Width" << YAML::Value << screenDevice->getWidth();
                        out << YAML::Key << "Height" << YAML::Value << screenDevice->getHeight();
                        out << YAML::EndMap;
                        break;
                    }
                    case DISK: {
                        const auto& diskDevice = std::dynamic_pointer_cast<CPU::Core::DiskDevice>(device);
                        out << YAML::Key << "Disk" << YAML::Value << YAML::BeginMap;
                        out << YAML::Key << "DiskId" << YAML::Value << diskDevice->GetCurrentDiskId();
                        out << YAML::Key << "ReadOnly" << YAML::Value << diskDevice->IsReadOnly();
                        out << YAML::EndMap;
                        break;
                    }
                    case AUDIO: {
                        const auto& audioDevice = std::dynamic_pointer_cast<CPU::Core::AudioDevice>(device);
                        out << YAML::Key << "MasterVolume" << YAML::Value << audioDevice->GetMasterVolume();
                        break;
                    }
                    case ENGINE: {
                        const auto& engineDevice = std::dynamic_pointer_cast<CPU::Core::CpuEngine>(device);
                        out << YAML::Key << "ConnectedCpu" << YAML::Value << YAML::BeginSeq;
                        for (const auto& interruptService: engineDevice->GetInterruptServices()) {
                            out << interruptService->deviceUUID;
                        }
                        out << YAML::EndSeq;
                    }
                    default:
                        break;
                }

                out << YAML::EndMap;
            }

            out << YAML::EndMap;
            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("DeviceLoader: save parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("DeviceLoader: save error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("DeviceLoader: saveValues end");
        return true;
    }
}
