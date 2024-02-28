//
// Created by pierr on 05/09/2023.
//

#include "NodesLoader.h"

#include "Commons/Log.h"
#include "Commons/Profiling.h"
#include "Commons/utils/YAMLimport.h"
#include "Commons/AstraException.h"
#include "Commons/utils/UUID.h"
#include "imnodes.h"

namespace Astra::UI::App {
    bool NodesLoader::loadConfig(const std::filesystem::path& pFilename, ProjectData& projectData) noexcept {
        LOG_CPU_DEBUG("[NodesLoader] loadConfig {}", pFilename);
        ENGINE_PROFILE_FUNCTION();

        if (!std::filesystem::exists(pFilename)) {
            LOG_CPU_DEBUG("[NodesLoader] nodes config file not found, skipping.");
            return true;
        }

        auto& nodesData = projectData.nodePanel;

        try {
            YAML::Node data = YAML::LoadFile(pFilename.string());
            if (!data["Version"] || !data["Nodes"] || !data["Info"] || !data["NodeCurrentIndex"]) {
                throw AstraException("invalid nodes config file: {}", pFilename);
            }

            if (data["Version"].as<double>() > ASTRA_VERSION) {
                throw AstraException("Invalid version in file {}", pFilename);
            }

            LOG_CPU_TRACE("[NodesLoader] Starting loading");

            nodesData.nodeId.clear();
            nodesData.nodeCurrentIndex = data["NodeCurrentIndex"].as<int>(1);

            const auto nodes = data["Nodes"];
            for (const auto node: nodes) {
                const auto nodeUUID = node.first.as<UUID>();
                const auto nodeIdx = node.second.as<int>();

                nodesData.nodeId[nodeUUID] = nodeIdx;
            }

            auto info = data["Info"].as<std::string>();
            ImNodes::LoadCurrentEditorStateFromIniString(info.c_str(), info.size());
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("NodesLoader: load parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("NodesLoader: load error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("NodesLoader: loadConfig end");
        return true;
    }

    bool NodesLoader::saveConfig(const std::filesystem::path& pFilename, const ProjectData& projectData) noexcept {
        LOG_CPU_DEBUG("[NodesLoader]: saveValues {}", pFilename);

        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Version" << YAML::Value << ASTRA_VERSION;
            out << YAML::Key << "NodeCurrentIndex" << YAML::Value << projectData.nodePanel.nodeCurrentIndex;

            out << YAML::Key << "Nodes" << YAML::Value << YAML::BeginMap;
            for (const auto& [nodeUUID, nodeIdx]: projectData.nodePanel.nodeId) {
                out << YAML::Key << nodeUUID << YAML::Value << nodeIdx;
            }
            out << YAML::EndMap;

            auto info = ImNodes::SaveCurrentEditorStateToIniString();
            out << YAML::Key << "Info" << YAML::Value << info;

            out << YAML::EndMap;

            std::ofstream fileOut(pFilename);
            AstraException::assertV(fileOut.good(), "can not open file {}", pFilename);
            fileOut << out.c_str();
            fileOut.close();
        } catch (const YAML::ParserException& e) {
            LOG_CPU_WARN("NodesLoader: save parser error: {}", e.msg);
            return false;
        } catch (const std::exception& e) {
            LOG_CPU_WARN("NodesLoader: save error {}", e.what());
            return false;
        }

        LOG_CPU_DEBUG("NodesLoader: saveValues end");
        return true;
    }
} // Astra