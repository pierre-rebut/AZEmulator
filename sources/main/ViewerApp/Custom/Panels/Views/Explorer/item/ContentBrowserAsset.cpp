//
// Created by pierr on 10/08/2023.
//
#include "ContentBrowserAsset.h"
#include "Commons/Log.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/Custom/Panels/Views/ExplorerPanel.h"
#include "ViewerApp/Custom/CustomEvents/EditorEvents.h"
#include "Commons/utils/Utils.h"
#include "ViewerApp/CoreLib/Utils.h"

namespace Astra::UI::App {
    ContentBrowserAsset::ContentBrowserAsset(const UI::Core::AssetMetadata& assetInfo, const Ref<UI::Core::Texture>& icon)
            : ContentBrowserItem(ContentBrowserItem::ItemType::Asset, assetInfo.Handle,
                                 assetInfo.FilePath.stem().string(), icon), m_AssetInfo(assetInfo) {
    }

    void ContentBrowserAsset::Delete() {
        auto filepath = UI::Core::AssetMetadata::GetFileSystemPath(m_AssetInfo);
        bool deleted = UI::Core::FileManager::DeleteFile(filepath);
        if (!deleted) {
            LOG_ERROR("Couldn't delete {0}", m_AssetInfo.FilePath);
            return;
        }

        auto currentDirectory = ExplorerPanel::Get().GetDirectory(m_AssetInfo.FilePath.parent_path());
        currentDirectory->Files.erase(std::remove(currentDirectory->Files.begin(), currentDirectory->Files.end(),
                                                  m_AssetInfo.Handle),
                                      currentDirectory->Files.end());

        UI::Core:: AssetManager::Get().OnAssetDeleted(m_AssetInfo.Handle);
    }

    bool ContentBrowserAsset::Move(const std::filesystem::path& destination) {
        auto filepath = UI::Core::AssetMetadata::GetFileSystemPath(m_AssetInfo);
        bool wasMoved = UI::Core::FileManager::MoveFile(filepath,
                                                  AstraProject::CurrentProject()->rootDirectory / destination);
        if (!wasMoved) {
            LOG_ERROR("Couldn't move {0} to {1}", m_AssetInfo.FilePath, destination);
            return false;
        }

        UI::Core::AssetManager::Get().OnAssetRenamed(m_AssetInfo.Handle, destination / filepath.filename());
        return true;
    }

    static const std::vector<UI::Core::AssetType> editorType = {
            UI::Core::AssetType::TEXT,
            UI::Core::AssetType::CONFIG,
            UI::Core::AssetType::ASM,
            UI::Core::AssetType::CPP,
            UI::Core::AssetType::IMAGE
    };

    void ContentBrowserAsset::Activate(CBItemActionResult& actionResult) {
        if (std::ranges::find(editorType.begin(), editorType.end(), m_AssetInfo.Type) != editorType.end()) {
            UI::Core::Events::Get().OnEvent<EditorOpenEvent>(m_AssetInfo);
        } else if (m_AssetInfo.Type == UI::Core::AssetType::AUDIO) {
            actionResult.Set(ContentBrowserAction::OpenExternal, true);
        }
    }

    void ContentBrowserAsset::OnRenamed(const std::string& newName) {
        auto filepath = UI::Core::AssetMetadata::GetFileSystemPath(m_AssetInfo);
        const std::string extension = filepath.extension().string();
        std::filesystem::path newFilepath = myFormat("{}\\{}{}", filepath.parent_path().string(), newName,
                                                     extension);

        std::string targetName = myFormat("{}{}", newName, extension);
        if (Utils::ToLower(targetName) == Utils::ToLower(filepath.filename().string())) {
            UI::Core::FileManager::RenameFilename(filepath, "temp-rename");
            filepath = myFormat("{}\\temp-rename{}", filepath.parent_path().string(), extension);
        }

        if (UI::Core::FileManager::RenameFilename(filepath, newName)) {
            // Update UI::Core:: AssetManager with new name
            UI::Core:: AssetManager::Get().OnAssetRenamed(m_AssetInfo.Handle, newFilepath);
        } else {
            LOG_ERROR("Couldn't rename {0} to {1}!", filepath.filename().string(), newName);
        }
    }

    void ContentBrowserAsset::OnDragDropItem() const {
        const auto& assetMetadata = GetAssetInfo();
        switch (assetMetadata.Type) {
            case UI::Core::AssetType::CONFIG: {
                ImGui::SetDragDropPayload("config_payload", &m_ID, sizeof(UUID));
                break;
            }
            case UI::Core::AssetType::CPP: {
                ImGui::SetDragDropPayload("cpp_payload", &m_ID, sizeof(UUID));
                break;
            }
            default:
                ImGui::SetDragDropPayload("asset_payload", &m_ID, sizeof(UUID));
        }
    }

    void ContentBrowserAsset::PasteCopiedAssets(const std::filesystem::path& originalFilePath) {
        auto newFilePath = originalFilePath / m_AssetInfo.FilePath;
        auto filepath = UI::Core::GetUniquePath(newFilePath);
        AstraException::assertV(!std::filesystem::exists(filepath), "Verify failed");
        std::filesystem::copy_file(newFilePath, filepath);
    }
}
