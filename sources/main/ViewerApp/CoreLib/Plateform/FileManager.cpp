//
// Created by pierr on 10/03/2022.
//

//#include <shellapi.h>

#include <fstream>
#include "FileManager.h"

// lib
#include "tinyfiledialogs.h"
#include "Commons/format.h"

namespace Astra::UI::Core {

    std::string FileManager::OpenFile(const std::string& pName, const std::string& pDescription,
                                      const std::vector<const char*>& pFilters, int pAllowMultiple) {
        auto ret = tinyfd_openFileDialog(
                pName.c_str(),
                "",
                static_cast<int>(pFilters.size()),
                pFilters.data(),
                pDescription.c_str(),
                pAllowMultiple
        );
        if (ret) {
            return ret;
        }
        return {};
    }

    std::string FileManager::SaveFile(const std::string& pName, const std::string& pDescription,
                                      const std::vector<const char*>& pFilters, const std::string& pDefaultName) {
        auto ret = tinyfd_saveFileDialog(
                pName.c_str(),
                pDefaultName.c_str(),
                static_cast<int>(pFilters.size()),
                pFilters.data(),
                pDescription.c_str()
        );
        if (ret) {
            return ret;
        }
        return {};
    }

    std::string FileManager::OpenFolder(const std::string& pName, const std::string& pPath) {
        auto ret = tinyfd_selectFolderDialog(
                pName.c_str(),
                pPath.c_str()
        );

        if (ret) return ret;
        return {};
    }

    bool FileManager::CreateDirectoryA(const std::filesystem::path& directory) {
        return std::filesystem::create_directories(directory);
    }

    bool FileManager::CreateNewFile(const std::filesystem::path& filepath) {
        std::ofstream file(filepath);
        bool res = file.good();
        file.close();
        return res;
    }

    bool FileManager::Rename(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath) {
        return Move(oldFilepath, newFilepath);
    }

    bool FileManager::RenameFilename(const std::filesystem::path& oldFilepath, const std::string& newName) {
        std::filesystem::path newPath = myFormat("{}\\{}{}", oldFilepath.parent_path().string(), newName,
                                                 oldFilepath.extension().string());
        return Rename(oldFilepath, newPath);
    }

    bool FileManager::DeleteFile(const std::filesystem::path& filepath) {
        if (!std::filesystem::exists(filepath))
            return false;

        if (std::filesystem::is_directory(filepath))
            return std::filesystem::remove_all(filepath) > 0;
        return std::filesystem::remove(filepath);
    }

    bool FileManager::Move(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath) {
        if (std::filesystem::exists(newFilepath))
            return false;

        std::filesystem::rename(oldFilepath, newFilepath);
        return true;
    }

    bool FileManager::MoveFile(const std::filesystem::path& filepath, const std::filesystem::path& dest) {
        return Move(filepath, dest / filepath.filename());
    }

    bool FileManager::ShowFileInExplorer(const std::filesystem::path& path) {
        auto absolutePath = std::filesystem::canonical(path);
        if (!std::filesystem::exists(absolutePath))
            return false;

        std::string cmd = myFormat("explorer.exe /select,\"{}\"", absolutePath.string());
        system(cmd.c_str());
        return true;
    }

    bool FileManager::OpenDirectoryInExplorer(const std::filesystem::path& path) {
        auto absolutePath = std::filesystem::canonical(path);
        if (!std::filesystem::exists(absolutePath))
            return false;

        auto cmd = myFormat("explorer.exe \"{}\"", absolutePath.string());
        system(cmd.c_str());
        return true;
    }

    bool FileManager::OpenExternally(const std::filesystem::path& item) {
        //ShellExecute(nullptr, L"open", item.c_str(), nullptr, nullptr, 1);
        return true;
    }

    bool FileManager::Copy(const std::filesystem::path& oldFilepath, const std::filesystem::path& newFilepath) {
        if (std::filesystem::exists(newFilepath))
            return false;

        std::filesystem::copy(oldFilepath, newFilepath);
        return true;
    }

    bool FileManager::CopyFile2(const std::filesystem::path& filepath, const std::filesystem::path& dest) {
        return Copy(filepath, dest / filepath.filename());
    }

    bool FileManager::CopyDirectory(const std::filesystem::path& dir, const std::filesystem::path& dest) {
        for (const auto& dirEntry : std::filesystem::directory_iterator(dir)) {
            const auto newEntry = dest / dirEntry.path().filename();
            if (dirEntry.is_directory()) {
                CreateDirectoryIfNotExists(newEntry);
                CopyDirectory(dirEntry, newEntry);
            } else {
                std::filesystem::copy(dirEntry, newEntry);
            }
        }
        
        return true;
    }

    bool FileManager::CreateDirectoryIfNotExists(const std::filesystem::path& directory) {
        if (!std::filesystem::exists(directory)) {
            return CreateDirectoryA(directory);
        }

        return true;
    }
}
