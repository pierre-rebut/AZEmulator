//
// Created by pierr on 26/07/2023.
//
#include "ProjectLoader.h"

#include <fstream>

#include "ViewerApp/Custom/AstraProject.h"
#include "Commons/utils/YAMLimport.h"
#include "CpuEngine/manager/EngineManager.h"
#include "Commons/Log.h"

namespace Astra::UI::App {
    bool ProjectLoader::loadConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept {
        LOG_DEBUG("[ProjectLoader]: loadConfig {}", pFilename);

        if (!std::filesystem::exists(pFilename)) {
            LOG_WARN("[ProjectLoader] project config file not found, skipping.");
            return false;
        }

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["ProjectName"] || !data["Settings"] || !data["EditorFiles"]) {
                throw AstraException("Invalid project dir {}", project->rootDirectory.string());
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            project->setProjectName(data["ProjectName"].as<std::string>());

            const auto settings = data["Settings"];
            ProjectSettings& projectSettings = project->getSettings();
            projectSettings.ContentBrowserThumbnailSize = settings["ContentBrowserThumbnailSize"].as<int>(128);
            projectSettings.ContentBrowserShowAssetTypes = settings["ContentBrowserShowAssetTypes"].as<bool>(true);
            projectSettings.DisplaySecondaryEntities = settings["DisplaySecondaryEntities"].as<bool>(false);
            projectSettings.LogLevel = settings["LogLevel"].as<int>(0);
            projectSettings.LogAutoCleanOnRun = settings["LogAutoCleanOnRun"].as<bool>(false);
            projectSettings.autoSaving = settings["AutoSaving"].as<bool>(true);

            const auto editorFiles = data["EditorFiles"];
            auto& openedEditorFiles = project->GetData().openedEditorFiles;

            for (const auto files: editorFiles) {
                openedEditorFiles.emplace_back(files.as<UUID>());
            }
        } catch (const YAML::Exception& e) {
            LOG_WARN("[ProjectLoader]: parser loader error: {}", e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("[ProjectLoader]: loader error: {}", e.what());
            return false;
        }

        LOG_DEBUG("[ProjectLoader]: loadConfig END");
        return true;
    }

    bool ProjectLoader::saveConfig(const std::filesystem::path& pFilename, AstraProject* project) noexcept {
        LOG_DEBUG("[ProjectLoader]: saveValues {}", pFilename);

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "ProjectName" << YAML::Value << project->getProjectName();

            out << YAML::Key << "Settings" << YAML::Value << YAML::BeginMap;

            ProjectSettings& projectSettings = project->getSettings();
            out << YAML::Key << "ContentBrowserThumbnailSize" << YAML::Value << projectSettings.ContentBrowserThumbnailSize;
            out << YAML::Key << "ContentBrowserShowAssetTypes" << YAML::Value << projectSettings.ContentBrowserShowAssetTypes;
            out << YAML::Key << "DisplaySecondaryEntities" << YAML::Value << projectSettings.DisplaySecondaryEntities;
            out << YAML::Key << "LogLevel" << YAML::Value << projectSettings.LogLevel;
            out << YAML::Key << "LogAutoCleanOnRun" << YAML::Value << projectSettings.LogAutoCleanOnRun;
            out << YAML::Key << "AutoSaving" << YAML::Value << projectSettings.autoSaving;

            out << YAML::EndMap;

            out << YAML::Key << "EditorFiles" << YAML::Value << YAML::BeginSeq;
            for (const auto& fileUUID: project->GetData().openedEditorFiles) {
                out << fileUUID;
            }
            out << YAML::EndSeq;

            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::Exception& e) {
            LOG_WARN("[ProjectLoader]: parser save error: {}", e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("[ProjectLoader]: save error: {}", e.what());
            return false;
        }

        LOG_DEBUG("[ProjectLoader]: saveValues END");
        return true;
    }
}
