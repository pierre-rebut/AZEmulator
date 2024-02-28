//
// Created by pierr on 25/03/2023.
//
#include "AssetManager.h"

#include "Commons/Log.h"
#include "Commons/utils/Utils.h"
#include "Commons/utils/YAMLimport.h"

#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/CoreLib/Events/AssetEvents.h"

#include "ViewerApp/Custom/ViewerConstants.h"

#include <algorithm>
#include <fstream>

namespace Astra::UI::Core {
    void AssetManager::Init(const Project* pProject) {
        loadAssetRegistry(pProject->rootDirectory);
    }

    void AssetManager::Shutdown(const Project* pProject) {
        writeRegistryToFile(pProject->rootDirectory);

        m_assets.clear();
    }

    const AssetMetadata& AssetManager::GetMetadata(UUID pHandle) const {
        if (auto it = m_assets.find(pHandle); it != m_assets.end()) {
            return it->second;
        }

        return InvalidAsset;
    }

    const AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& pPath) const {
        auto it = std::ranges::find_if(m_assets.begin(), m_assets.end(),
                                       [&pPath](const AssetMapList::value_type& elem) {
                                           return elem.second.FilePath == pPath;
                                       });

        if (it != m_assets.end()) {
            return it->second;
        }

        return InvalidAsset;
    }

    const AssetMetadata& AssetManager::GetOrCreateMetadata(const std::filesystem::path& pPath) {
        const auto& metaData = GetMetadata(pPath);
        if (metaData.isValid()) {
            return metaData;
        }

        const auto assetType = AssetMetadata::GetAssetTypeFromPath(pPath);
        if (assetType == AssetType::UNKNOWN) {
            return InvalidAsset;
        }

        auto uuid = UUIDGen::New();
        m_assets[uuid] = AssetMetadata{pPath, uuid, assetType};
        writeRegistryToFile(Project::CurrentProject()->rootDirectory);

        return m_assets[uuid];
    }

    const AssetMetadata& AssetManager::CreateNewAsset(const std::string& filename, const std::filesystem::path& directoryPath) {
        AssetMetadata metadata;
        metadata.Handle = UUIDGen::New();

        if (directoryPath.empty() || directoryPath == ".")
            metadata.FilePath = filename;
        else
            metadata.FilePath = directoryPath / filename;

        metadata.Type = AssetMetadata::GetAssetTypeFromPath(metadata.FilePath);

        if (std::filesystem::exists(AssetMetadata::GetFileSystemPath(metadata))) {
            bool foundAvailableFileName = false;
            int current = 1;

            while (!foundAvailableFileName) {
                auto nextFilePath = Project::CurrentProject()->rootDirectory / directoryPath / metadata.FilePath.stem();

                if (current < 10)
                    nextFilePath += myFormat(" (0{})", current);
                else
                    nextFilePath += myFormat(" ({})", current);
                nextFilePath += metadata.FilePath.extension();

                if (!std::filesystem::exists(nextFilePath)) {
                    foundAvailableFileName = true;
                    metadata.FilePath = AssetMetadata::GetRelativePath(nextFilePath);
                }

                current++;
            }
        }

        m_assets[metadata.Handle] = metadata;
        writeRegistryToFile(Project::CurrentProject()->rootDirectory);

        FileManager::CreateNewFile(AssetMetadata::GetFileSystemPath(metadata));

        return m_assets[metadata.Handle];
    }

    void AssetManager::OnAssetDeleted(UUID pHandle) {
        const AssetMetadata& metadata = GetMetadata(pHandle);

        if (metadata.isValid()) {
            m_assets.erase(pHandle);
            Events::Get().OnEvent<AssetChangedEvent>(metadata, AssetChangedEvent::Event::ASSET_DELETE);
        }
    }

    void AssetManager::OnAssetRenamed(UUID assetHandle, const std::filesystem::path& newFilePath) {
        auto it = m_assets.find(assetHandle);
        if (it != m_assets.end()) {
            it->second.FilePath = AssetMetadata::GetRelativePath(newFilePath);
            Events::Get().OnEvent<AssetChangedEvent>(it->second, AssetChangedEvent::Event::ASSET_RENAMED);
        }
    }

    void AssetManager::loadAssetRegistry(const std::filesystem::path& projectDirectory) noexcept {
        const auto& assetRegistryPath = projectDirectory /App::ViewerConstants::ASTRA_DIR / App::ViewerConstants::REGISTRY_FILE;
        if (!std::filesystem::exists(assetRegistryPath)) {
            return;
        }

        LOG_INFO("[AssetManager] Loading Asset Registry");

        try {
            YAML::Node data = YAML::LoadFile(assetRegistryPath.string());
            auto handles = data["Assets"];
            if (!handles) {
                LOG_ERROR("[AssetManager] Asset Registry appears to be corrupted!");
                return;
            }

            for (auto entry: handles) {
                auto filepath = entry["FilePath"].as<std::string>();

                AssetMetadata metadata;
                metadata.Handle = entry["Handle"].as<uint64_t>();
                metadata.FilePath = filepath;
                metadata.Type = AssetMetadata::AssetTypeFromString(entry["Type"].as<std::string>());

                if (metadata.Type == AssetType::UNKNOWN) {
                    continue;
                }

                if (!std::filesystem::exists(projectDirectory / metadata.FilePath)) {
                    locateMissingFile(metadata, filepath, projectDirectory);
                }

                if (metadata.Handle == 0) {
                    LOG_WARN("[AssetManager] AssetHandle for {} is 0, this shouldn't happen.", metadata.FilePath);
                    continue;
                }

                m_assets[metadata.Handle] = metadata;
            }

            LOG_INFO("[AssetManager] Loaded {} asset entries", m_assets.size());
        } catch (const std::exception& e) {
            LOG_ERROR("[AssetManager] Load registry error: {}", e.what());
        }
    }

    bool AssetManager::locateMissingFile(AssetMetadata& metadata, const std::string_view& filepath,
                                         const std::filesystem::path& projectDirectory) {
        LOG_WARN("[AssetManager] Missing asset {} detected in registry file, trying to locate...", metadata.FilePath);

        std::string mostLikelyCandidate;
        uint32_t bestScore = 0;

        for (auto& pathEntry: std::filesystem::recursive_directory_iterator(projectDirectory)) {
            const std::filesystem::path& path = pathEntry.path();

            if (path.filename() != metadata.FilePath.filename()) {
                continue;
            }

            if (bestScore > 0)
                LOG_WARN("[AssetManager] Multiple candidates found...");

            std::vector<std::string> candidateParts = Utils::SplitString(path.string(), "/\\");

            uint32_t score = 0;
            for (const auto& part: candidateParts) {
                if (filepath.find(part) != std::string::npos) {
                    score++;
                }
            }

            LOG_WARN("{} has a score of {}, best score is {}", path, score, bestScore);

            if (bestScore > 0 && score == bestScore) {
                // TODO: How do we handle this?
                // Probably prompt the user at this point?
            }

            if (score <= bestScore) {
                continue;
            }

            bestScore = score;
            mostLikelyCandidate = path.string();
        }

        if (mostLikelyCandidate.empty() && bestScore == 0) {
            LOG_ERROR("[AssetManager] Failed to locate a potential match for {}", metadata.FilePath);
            return false;
        }

        std::ranges::replace(mostLikelyCandidate.begin(), mostLikelyCandidate.end(), '\\', '/');
        metadata.FilePath = std::filesystem::relative(mostLikelyCandidate, projectDirectory);

        LOG_WARN("[AssetManager] Found most likely match {}", metadata.FilePath);
        return true;
    }

    void AssetManager::writeRegistryToFile(const std::filesystem::path& projectDirectory) noexcept {
        try {
            // Sort assets by UUID to make project management easier
            struct AssetRegistryEntry
            {
                std::string FilePath;
                AssetType Type = AssetType::UNKNOWN;
            };

            std::map<UUID, AssetRegistryEntry> sortedMap;
            for (const auto& [filepath, metadata]: m_assets) {
                if (!std::filesystem::exists(projectDirectory / metadata.FilePath)) {
                    continue;
                }

                std::string pathToSerialize = metadata.FilePath.string();
                std::ranges::replace(pathToSerialize.begin(), pathToSerialize.end(), '\\', '/');
                sortedMap[metadata.Handle] = {pathToSerialize, metadata.Type};
            }

            LOG_INFO("[AssetManager] serializing asset registry with {0} entries", sortedMap.size());

            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Assets" << YAML::BeginSeq;
            for (const auto& [handle, entry]: sortedMap) {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << handle;
                out << YAML::Key << "FilePath" << YAML::Value << entry.FilePath;
                out << YAML::Key << "Type" << YAML::Value << AssetMetadata::AssetTypeToString(entry.Type);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream fout(projectDirectory / App::ViewerConstants::ASTRA_DIR / App::ViewerConstants::REGISTRY_FILE);
            fout << out.c_str();
            fout.close();
        } catch (const std::exception& e) {
            LOG_ERROR("[AssetManager] Save registry error: {}", e.what());
        }
    }
}
