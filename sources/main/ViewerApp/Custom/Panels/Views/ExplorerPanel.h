//
// Created by pierr on 06/03/2022.
//

#pragma once

#include <filesystem>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/item/ContentBrowserItem.h"
#include "Commons/utils/Singleton.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/item/ContentBrowserDirectory.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/ContentBrowserItemList.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/SelectionStack.h"
#include "ViewerApp/CoreLib/Windows/BasicPopups/SimpleQuestionModal.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class ExplorerPanel : public UI::Core::APanel, public Singleton<ExplorerPanel>
    {
    public:
        static constexpr const char* NAME = ICON_FA_FOLDER_OPEN " Explorer";

        Ref<DirectoryInfo> GetDirectory(const std::filesystem::path& filepath) const;

        ContentBrowserItemList& GetCurrentItems() { return m_CurrentItems; }

        void OnProjectChanged();

    private:
        static std::mutex s_LockMutex;
        UI::Core::SimpleQuestionModal m_questionModal{"Explorer"};

        AstraProject* m_project = nullptr;

        bool m_IsAnyItemHovered = false;
        bool m_IsContentBrowserHovered = false;
        bool m_IsContentBrowserFocused = false;

        Ref<DirectoryInfo> m_CurrentDirectory;
        Ref<DirectoryInfo> m_BaseDirectory;
        Ref<DirectoryInfo> m_NextDirectory;
        Ref<DirectoryInfo> m_PreviousDirectory;

        std::unordered_map<UUID, Ref<DirectoryInfo>> m_Directories;
        std::unordered_map<Core::AssetType, Ref<UI::Core::Texture>> m_AssetIconMap;

        ContentBrowserItemList m_CurrentItems;

        SelectionStack m_CopiedAssets;

        char m_SearchBuffer[ContentBrowserItem::MAX_INPUT_BUFFER_LENGTH] = {0};

        std::vector<Ref<DirectoryInfo>> m_BreadCrumbData;
        bool m_UpdateNavigationPath = false;

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override;

        void OnEvent(UI::Core::AEvent& pEvent) override;

        void drawPanelContent() override;

        void OnBrowseBack();

        void OnBrowseForward();

        void UpdateDropArea(const Ref<DirectoryInfo>& target);

        void ChangeDirectory(const Ref<DirectoryInfo>& directory);

        void RemoveDirectory(const Ref<DirectoryInfo>& directory, bool removeFromParent = true);

        bool checkIfAnyDescendantSelected(const Ref<DirectoryInfo>& directory);

        void RenderTopBar();

        void RenderItems();

        void RenderBottomBar();

        void RenderDirectoryHierarchy(const Ref<DirectoryInfo>& directory);

        bool renderItemAction(const Ref<ContentBrowserItem>& item);

        void RenderDeleteDialogue();

        void Refresh();

        void RefreshWithoutLock();

        void UpdateInput() const;

        void PasteCopiedAssets();

        void ClearSelections() const;

        void SortItemList();

        ContentBrowserItemList Search(const std::string& query, const Ref<DirectoryInfo>& directoryInfo);

        UUID ProcessDirectory(const std::filesystem::path& directoryPath, const Ref<DirectoryInfo>& parent);

    public:
        ExplorerPanel();

        void contentOutliner();

        void contentDirectory();

        void drawContentPopup();

    private:
        void drawDirectoryContent();
        const UI::Core::AssetMetadata& CreateAssetInDirectory(const std::string& filename, const Ref<DirectoryInfo>& directory);
        void drawContextMenu(const Ref<DirectoryInfo>& directory);
        void buildItem(UUID id) const;
    };

}
