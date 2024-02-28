//
// Created by pierr on 25/03/2023.
//
#pragma once

#include <unordered_map>
#include <filesystem>

#include "Commons/utils/UUID.h"
#include "AssetMetadata.h"
#include "Commons/utils/Singleton.h"

namespace Astra::UI::Core {

    using AssetMapList = std::unordered_map<UUID, AssetMetadata>;

    class AssetManager : public Singleton<AssetManager>
    {
    public:
        static constexpr const char* NAME = "AssetManager";
    private:
        inline static AssetMetadata InvalidAsset{"", 0, AssetType::UNKNOWN};

        AssetMapList m_assets;

        void loadAssetRegistry(const std::filesystem::path& projectDirectory) noexcept;

        static bool locateMissingFile(AssetMetadata& metadata, const std::string_view& filepath,
                                      const std::filesystem::path& projectDirectory);

        void writeRegistryToFile(const std::filesystem::path& projectDirectory) noexcept;

    public:
        void Init(const Project*);

        void Shutdown(const Project*);

        void Reset() {m_assets.clear();}

        const AssetMetadata& GetMetadata(UUID pHandle) const;
        const AssetMetadata& GetMetadata(const std::filesystem::path& pPath) const;

        const AssetMetadata& GetOrCreateMetadata(const std::filesystem::path& pPath);

        const AssetMapList& GetAssetsList() const {return m_assets;}

        const AssetMetadata& CreateNewAsset(const std::string& filename, const std::filesystem::path& directoryPath = "");

        void OnAssetDeleted(UUID pHandle);

        void OnAssetRenamed(UUID assetHandle, const std::filesystem::path& newFilePath);
    };

}
