//
// Created by pierr on 28/03/2023.
//
#pragma once

#include <cstdint>
#include <array>
#include <filesystem>
#include "Commons/utils/UUID.h"
#include "ViewerApp/CoreLib/Project.h"

namespace Astra::UI::Core {

    enum class AssetType : uint16_t
    {
        UNKNOWN = 0,
        TEXT,
        CONFIG,
        BINARY,
        ASM,
        CPP,
        REG_DUMP,
        MEM_DUMP,
        IMAGE,
        FONT,
        AUDIO,
        DISK
    };

    struct AssetMetadata
    {
        std::filesystem::path FilePath;
        UUID Handle;
        AssetType Type;

        bool isValid() const {
            return Handle != 0;
        }

        inline static std::filesystem::path GetFileSystemPath(const AssetMetadata& metadata) {
            return Project::CurrentProject()->rootDirectory / metadata.FilePath;
        }

        inline static std::string GetFileSystemPathString(const AssetMetadata& metadata) {
            return GetFileSystemPath(metadata).string();
        }

        static std::filesystem::path GetRelativePath(const std::filesystem::path& filepath);

        static AssetType GetAssetTypeFromExtension(const std::string& pExt);

        inline static AssetType GetAssetTypeFromPath(const std::filesystem::path& path) {
            return GetAssetTypeFromExtension(path.extension().string());
        }

        static const char* AssetTypeToString(const AssetType& pType);

        static AssetType AssetTypeFromString(const std::string_view& pType);

        inline static const std::pair<std::string, AssetType> s_AssetExtensionMap[] = {
                {".txt",  AssetType::TEXT},
                {".md",  AssetType::TEXT},
                {".yml",  AssetType::CONFIG},
                {".cfg",  AssetType::CONFIG},
                {".conf",  AssetType::CONFIG},
                {".cpp",  AssetType::CPP},

                {".reg",  AssetType::REG_DUMP},
                {".mem",  AssetType::MEM_DUMP},

                {".bin",  AssetType::BINARY},
                {".dll",  AssetType::BINARY},
                {".asm",  AssetType::ASM},
                {".s",  AssetType::ASM},

                // Image
                {".png",  AssetType::IMAGE},
                {".jpg",  AssetType::IMAGE},
                {".jpeg", AssetType::IMAGE},
                {".rgb",  AssetType::IMAGE},

                // Audio
                {".wav",  AssetType::AUDIO},
                {".ogg",  AssetType::AUDIO},


                {".dsk",  AssetType::DISK},

                // Fonts
                {".ttf",  AssetType::FONT},
                {".ttc",  AssetType::FONT},
                {".otf",  AssetType::FONT},
        };
    };
}
