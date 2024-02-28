//
// Created by pierr on 10/08/2023.
//

#include "ContentBrowserDirectory.h"
#include "Commons/Log.h"
#include "ViewerApp/Custom/Panels/Views/ExplorerPanel.h"
#include "ViewerApp/Custom/Utils/SelectionManager.h"
#include "Commons/utils/Utils.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/CoreLib/Utils.h"

namespace Astra::UI::App {
    ContentBrowserDirectory::ContentBrowserDirectory(const Ref<DirectoryInfo>& directoryInfo)
            : ContentBrowserItem(ContentBrowserItem::ItemType::Directory, directoryInfo->Handle,
                                 directoryInfo->FilePath.filename().string(), ViewerResources::FolderIcon),
              m_DirectoryInfo(directoryInfo) {
    }

    void ContentBrowserDirectory::Activate(CBItemActionResult& actionResult) {
        actionResult.Set(ContentBrowserAction::NavigateToThis, true);
    }

    void ContentBrowserDirectory::OnRenamed(const std::string& newName) {
        const std::filesystem::path& mainDirectory = AstraProject::CurrentProject()->rootDirectory;

        auto target = mainDirectory / m_DirectoryInfo->FilePath;
        auto destination = mainDirectory / m_DirectoryInfo->FilePath.parent_path() / newName;

        if (Utils::ToLower(newName) == Utils::ToLower(target.filename().string())) {
            auto tmp = mainDirectory / m_DirectoryInfo->FilePath.parent_path() / "TempDir";
            UI::Core::FileManager::Rename(target, tmp);
            target = tmp;
        }

        if (!UI::Core::FileManager::Rename(target, destination)) {
            LOG_ERROR("Couldn't rename {0} to {1}!", m_DirectoryInfo->FilePath.filename().string(), newName);
        }
    }

    static const ImGuiPayload* findDragDropPayload() {
        if (auto payload = ImGui::AcceptDragDropPayload("assets_payload")) {
            return payload;
        }
        if (auto payload = ImGui::AcceptDragDropPayload("asset_payload")) {
            return payload;
        }
        if (auto payload = ImGui::AcceptDragDropPayload("config_payload")) {
            return payload;
        }
        if (auto payload = ImGui::AcceptDragDropPayload("cpp_payload")) {
            return payload;
        }

        return nullptr;
    }

    void ContentBrowserDirectory::UpdateDrop(CBItemActionResult& actionResult) {
        if (SelectionManager::IsSelected(SelectionContext::ContentBrowser, m_ID)) {
            return;
        }

        if (ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = findDragDropPayload();

            if (payload) {
                dragDropMove(actionResult, payload);
            }

            ImGui::EndDragDropTarget();
        }
    }

    void ContentBrowserDirectory::dragDropMove(CBItemActionResult& actionResult, const ImGuiPayload* payload) const {
        auto& currentItems = ExplorerPanel::Get().GetCurrentItems();
        uint32_t count = payload->DataSize / sizeof(UUID);

        for (uint32_t i = 0; i < count; i++) {
            UUID assetHandle = *(((UUID*) payload->Data) + i);
            size_t index = currentItems.FindItem(assetHandle);
            if (index != ContentBrowserItemList::InvalidItem) {
                if (currentItems[index]->Move(m_DirectoryInfo->FilePath)) {
                    actionResult.Set(ContentBrowserAction::Refresh, true);
                    currentItems.erase(assetHandle);
                }
            }
        }
    }

    void ContentBrowserDirectory::Delete() {
        bool deleted = UI::Core::FileManager::DeleteFile(
                AstraProject::CurrentProject()->rootDirectory / m_DirectoryInfo->FilePath);
        if (!deleted) {
            LOG_WARN("Failed to delete folder {0}", m_DirectoryInfo->FilePath);
            return;
        }

        for (auto asset: m_DirectoryInfo->Files) {
            UI::Core:: AssetManager::Get().OnAssetDeleted(asset);
        }
    }

    bool ContentBrowserDirectory::Move(const std::filesystem::path& destination) {
        const std::filesystem::path& mainDirectory = AstraProject::CurrentProject()->rootDirectory;
        bool wasMoved = UI::Core::FileManager::MoveFile(
                mainDirectory / m_DirectoryInfo->FilePath,
                mainDirectory / destination
        );
        if (!wasMoved) {
            return false;
        }

        return true;
    }

    void ContentBrowserDirectory::PasteCopiedAssets(const std::filesystem::path& originalFilePath) {
        const auto newFilePath = originalFilePath / m_DirectoryInfo->FilePath;
        auto filepath = UI::Core::GetUniquePath(newFilePath);
        AstraException::assertV(!std::filesystem::exists(filepath), "Verify failed");
        std::filesystem::copy(newFilePath, filepath, std::filesystem::copy_options::recursive);
    }
}
