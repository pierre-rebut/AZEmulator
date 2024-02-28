//
// Created by pierr on 29/07/2023.
//

#include "LogLoader.h"

#include <fstream>

#include "Commons/Log.h"
#include "Commons/utils/YAMLimport.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "CpuEngine/manager/EngineManager.h"

namespace Astra::UI::App {
    bool LogLoader::loadConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept {
        LOG_DEBUG("[LogLoader]: loadConfig {}", pFilename);

        if (!std::filesystem::exists(pFilename)) {
            LOG_DEBUG("LogLoader: file not found, skipped.");
            return true;
        }

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["Logs"]) {
                throw AstraException("Invalid logs file {}", pFilename.string());
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            auto& logMessages = project->GetData().logMessages;

            for (const auto log: data["Logs"]) {
                auto msg = AstraMessage::New(AstraMessage::convertTypeFromString(log["Type"].as<std::string>("info")));
                msg->setTitle(log["Title"].as<std::string>(""));
                msg->setContent(log["Content"].as<std::string>(""));
                msg->setTiming(std::chrono::system_clock::from_time_t(log["Time"].as<std::time_t>(0)));
                logMessages.emplace_back(msg);
            }
        } catch (const YAML::Exception& e) {
            LOG_WARN("[LogLoader]: parser loader error: {}", e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("[LogLoader]: loader error: {}", e.what());
            return false;
        }

        return true;
    }

    bool LogLoader::saveConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept {
        LOG_DEBUG("[ProjectLoader]: saveValues {}", pFilename);

        try {
            auto& logMessages = project->GetData().logMessages;

            CPU::Core::LogManager::Get().RetrieveMessages(logMessages);

            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "Logs" << YAML::Value << YAML::BeginSeq;

            for (const auto& msg: logMessages) {
                out << YAML::BeginMap;

                out << YAML::Key << "Type" << YAML::Value << AstraMessage::convertTypeToString(msg->type);
                out << YAML::Key << "Title" << YAML::Value << msg->getTitle();
                out << YAML::Key << "Content" << YAML::Value << msg->getContent();
                out << YAML::Key << "Time" << YAML::Value << std::chrono::system_clock::to_time_t(msg->getTiming());

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;

            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::Exception& e) {
            LOG_WARN("[LogLoader]: parser save error: {}", e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("[LogLoader]: save error: {}", e.what());
            return false;
        }

        return true;
    }
}
