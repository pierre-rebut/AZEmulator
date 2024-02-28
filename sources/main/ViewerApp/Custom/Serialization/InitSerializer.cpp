//
// Created by pierr on 16/01/2024.
//
#include <filesystem>
#include "InitSerializer.h"

#include "Commons/Log.h"
#include "Commons/utils/YAMLimport.h"
#include "ViewerApp/Custom/ViewerConstants.h"
#include "Commons/AstraException.h"

namespace Astra::UI::App {

    void InitSerializer::load(std::list<Ref<ProjectHistory>>& projectsHistory) noexcept {
        LOG_DEBUG("InitModal: loadProjectsHistory");

        try {
            YAML::Node data = YAML::LoadFile(ViewerConstants::CONFIG_RECENTLY_PROJECT_FILE);
            if (!data["Version"] || !data["Projects"]) {
                throw AstraException("InitModal: invalid projects history file: {}", ViewerConstants::CONFIG_RECENTLY_PROJECT_FILE);
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", ViewerConstants::CONFIG_RECENTLY_PROJECT_FILE);
            }

            projectsHistory.clear();

            auto projects = data["Projects"];
            for (const auto project: projects) {
                if (!project["Name"] || !project["Path"]) {
                    LOG_WARN("InitModal: invalid project, skipping");
                    continue;
                }

                auto projectName = project["Name"].as<std::string>();
                std::filesystem::path projectPath = project["Path"].as<std::string>();

                if (!std::filesystem::exists(projectPath)) {
                    LOG_WARN("InitModal: invalid project path {}, skipping", projectPath);
                    continue;
                }

                projectsHistory.emplace_back(CreateRef<ProjectHistory>(projectName, projectPath));
            }

        } catch (const YAML::ParserException& e) {
            LOG_WARN("InitModal: load parser error: {}", e.msg);
        } catch (const std::exception& e) {
            LOG_WARN("InitModal: load error {}", e.what());
        }

        LOG_DEBUG("InitModal: loadProjectsHistory END");
    }

    void InitSerializer::update(const std::list<Ref<ProjectHistory>>& projectsHistory) noexcept {
        LOG_DEBUG("InitSerializer: UpdateProjectsHistory");

        try {
            std::ofstream fileOut(ViewerConstants::CONFIG_RECENTLY_PROJECT_FILE);
            AstraException::assertV(fileOut.good(), "InitModal: can not open file {}",
                                    ViewerConstants::CONFIG_RECENTLY_PROJECT_FILE);

            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "Projects" << YAML::Value << YAML::BeginSeq;

            for (const auto& projectInfo: projectsHistory) {
                out << YAML::BeginMap;
                out << YAML::Key << "Name" << YAML::Value << projectInfo->name;
                out << YAML::Key << "Path" << YAML::Value << projectInfo->path.string();
                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
            out << YAML::EndMap;

            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::ParserException& e) {
            LOG_WARN("InitModal: save parser error: {}", e.msg);
        } catch (const std::exception& e) {
            LOG_WARN("InitModal: save error {}", e.what());
        }

        LOG_DEBUG("InitModal: UpdateProjectsHistory END");
    }
}
