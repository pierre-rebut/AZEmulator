//
// Created by pierr on 28/03/2023.
//

#include "AssetMetadata.h"
#include "Commons/utils/Utils.h"

namespace Astra::UI::Core {

    AssetType AssetMetadata::GetAssetTypeFromExtension(const std::string& pExt) {
        std::string lowerExt = Utils::ToLower(pExt);
        for (const auto& [ext, val]: AssetMetadata::s_AssetExtensionMap) {
            if (lowerExt == ext) {
                return val;
            }
        }

        return AssetType::UNKNOWN;
    }

    std::filesystem::path AssetMetadata::GetRelativePath(const std::filesystem::path& filepath) {
        std::filesystem::path relativePath = filepath.lexically_normal();
        std::string temp = filepath.string();

        const std::filesystem::path& mainDirectory = Project::CurrentProject()->rootDirectory;
        if (temp.find(mainDirectory.string()) != std::string::npos) {
            relativePath = std::filesystem::relative(filepath, mainDirectory);
            if (relativePath.empty()) {
                relativePath = filepath.lexically_normal();
            }
        }
        return relativePath;
    }

    const char* AssetMetadata::AssetTypeToString(const AssetType& pType) {
        switch (pType) {
            using
            enum AssetType;
            case UNKNOWN:
                return "UNKNOWN";
            case TEXT:
                return "TEXT";
            case CONFIG:
                return "CONFIG";
            case BINARY:
                return "BINARY";
            case CPP:
                return "CPP";
            case ASM:
                return "ASM";
            case REG_DUMP:
                return "REG";
            case MEM_DUMP:
                return "MEM";
            case IMAGE:
                return "IMAGE";
            case FONT:
                return "FONT";
            case AUDIO:
                return "AUDIO";
            case DISK:
                return "DISK";
        }
        return nullptr;
    }

    AssetType AssetMetadata::AssetTypeFromString(const std::string_view& pType) {
        using
        enum AssetType;
        if (pType == "TEXT") return TEXT;
        if (pType == "CONFIG") return CONFIG;
        if (pType == "BINARY") return BINARY;
        if (pType == "CPP") return CPP;
        if (pType == "ASM") return ASM;
        if (pType == "REG") return REG_DUMP;
        if (pType == "MEM") return MEM_DUMP;
        if (pType == "IMAGE") return IMAGE;
        if (pType == "FONT") return FONT;
        if (pType == "AUDIO") return AUDIO;
        if (pType == "DISK") return DISK;

        return UNKNOWN;
    }
}
