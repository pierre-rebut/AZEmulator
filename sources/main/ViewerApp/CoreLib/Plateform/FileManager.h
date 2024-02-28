//
// Created by pierr on 10/03/2022.
//

#pragma once

// std
#include <vector>
#include <string>
#include <filesystem>

#ifdef CreateDirectory
#undef CreateDirectory
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef SetEnvironmentVariable
#undef GetEnvironmentVariable
#endif

namespace Astra::UI::Core {
    class FileManager
    {
    public:
        FileManager() = delete;

        static bool CreateDirectoryA(const std::filesystem::path& directory);
        static bool CreateDirectoryIfNotExists(const std::filesystem::path& directory);
        static bool CreateNewFile(const std::filesystem::path& filepath);
        static bool Rename(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath);
        static bool RenameFilename(const std::filesystem::path& oldFilepath, const std::string& newName);
        static bool DeleteFile(const std::filesystem::path& filepath);
        static bool Move(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath);
        static bool MoveFile(const std::filesystem::path& filepath, const std::filesystem::path& dest);
        static bool Copy(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath);
        static bool CopyFile2(const std::filesystem::path& filepath, const std::filesystem::path& dest);
        static bool CopyDirectory(const std::filesystem::path& dir, const std::filesystem::path& dest);

        // These return empty strings if cancelled
        static std::string
        OpenFile(const std::string& pName, const std::string& pDescription, const std::vector<const char*>& pFilters,
                 int pAllowMultiple = 0);

        static std::string
        SaveFile(const std::string& pName, const std::string& pDescription, const std::vector<const char*>& pFilters,
                 const std::string& pDefaultName);

        static std::string OpenFolder(const std::string& pName, const std::string& pPath);

        static bool ShowFileInExplorer(const std::filesystem::path& path);
        static bool OpenDirectoryInExplorer(const std::filesystem::path& path);
        static bool OpenExternally(const std::filesystem::path& item);
    };
}
