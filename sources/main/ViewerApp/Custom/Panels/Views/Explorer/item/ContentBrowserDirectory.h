//
// Created by pierr on 10/08/2023.
//
#pragma once

#include "ContentBrowserItem.h"

namespace Astra::UI::App {

    struct DirectoryInfo
    {
        UUID Handle;
        Ref<DirectoryInfo> Parent;
        std::filesystem::path FilePath;
        std::vector<UUID> Files;
        std::map<UUID, Ref<DirectoryInfo>> SubDirectories;
    };

    class ContentBrowserDirectory : public ContentBrowserItem
    {
    public:
        explicit ContentBrowserDirectory(const Ref<DirectoryInfo>& directoryInfo);

        Ref<DirectoryInfo>& GetDirectoryInfo() { return m_DirectoryInfo; }

        void Delete() override;
        bool Move(const std::filesystem::path& destination) override;
        void PasteCopiedAssets(const std::filesystem::path& originalFilePath) override;

    private:
        void Activate(CBItemActionResult& actionResult) override;
        void OnRenamed(const std::string& newName) override;
        void UpdateDrop(CBItemActionResult& actionResult) override;

        void UpdateDirectoryPath(Ref<DirectoryInfo> directoryInfo, const std::filesystem::path& newParentPath, const std::filesystem::path& newName);

    private:
        Ref<DirectoryInfo> m_DirectoryInfo;
        void dragDropMove(CBItemActionResult& actionResult, const ImGuiPayload* payload) const;
    };

}
