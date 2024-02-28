//
// Created by pierr on 26/07/2023.
//
#include "AppLoader.h"

#include <fstream>

#include "ViewerApp/Custom/AstraProject.h"
#include "Commons/utils/YAMLimport.h"
#include "CpuEngine/manager/EngineManager.h"
#include "Commons/Log.h"

namespace Astra::UI::App {
    bool AppLoader::loadConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept {
        LOG_DEBUG("[AppLoader]: loadConfig {}", pFilename);

        if (!std::filesystem::exists(pFilename)) {
            LOG_WARN("[AppLoader] project config file not found, skipping.");
            return false;
        }

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["Settings"]) {
                throw AstraException("Invalid project dir {}", project->rootDirectory.string());
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            const auto settings = data["Settings"];
            ProjectSettings& projectSettings = project->getSettings();
            projectSettings.CpuSpeed = settings["CpuSpeed"].as<int>(500);
            projectSettings.screenUUID = settings["ScreenUUID"].as<uint64_t>(0);
            projectSettings.keyboardUUID = settings["KeyboardUUID"].as<uint64_t>(0);
            projectSettings.mouseUUID = settings["MouseUUID"].as<uint64_t>(0);
            projectSettings.audioUUID = settings["AudioUUID"].as<uint64_t>(0);
            projectSettings.serialUUID = settings["SerialUUID"].as<uint64_t>(0);

        } catch (const YAML::Exception& e) {
            LOG_WARN("[AppLoader]: parser loader error: {}", e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("[AppLoader]: loader error: {}", e.what());
            return false;
        }

        LOG_DEBUG("[AppLoader]: loadConfig END");
        return true;
    }

    bool AppLoader::saveConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept {
        LOG_DEBUG("[AppLoader]: saveValues {}", pFilename);

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;

            out << YAML::Key << "Settings" << YAML::Value << YAML::BeginMap;

            ProjectSettings& projectSettings = project->getSettings();
            out << YAML::Key << "CpuSpeed" << YAML::Value << projectSettings.CpuSpeed;
            out << YAML::Key << "ScreenUUID" << YAML::Value << projectSettings.screenUUID;
            out << YAML::Key << "KeyboardUUID" << YAML::Value << projectSettings.keyboardUUID;
            out << YAML::Key << "MouseUUID" << YAML::Value << projectSettings.mouseUUID;
            out << YAML::Key << "AudioUUID" << YAML::Value << projectSettings.audioUUID;
            out << YAML::Key << "SerialUUID" << YAML::Value << projectSettings.serialUUID;

            out << YAML::EndMap;

            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::Exception& e) {
            LOG_WARN("[AppLoader]: parser save error: {}", e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("[AppLoader]: save error: {}", e.what());
            return false;
        }

        LOG_DEBUG("[AppLoader]: saveValues END");
        return true;
    }
}
