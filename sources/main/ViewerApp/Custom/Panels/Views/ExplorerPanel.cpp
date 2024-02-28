//
// Created by pierr on 06/03/2022.
//

#include "ExplorerPanel.h"

#include "Commons/Profiling.h"
#include "Commons/utils/Utils.h"

#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/Custom/ViewerResources.h"
#include "ViewerApp/Custom/Utils/SelectionManager.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/item/ContentBrowserAsset.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/item/ContentBrowserDirectory.h"

#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "ViewerApp/CoreLib/Resources/Widgets.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/CoreLib/Resources/CustomTreeNode.h"
#include "ViewerApp/CoreLib/Resources/Resources.h"

#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"

#include "imgui.h"
#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {
    std::mutex ExplorerPanel::s_LockMutex;

    ExplorerPanel::ExplorerPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {
        ExplorerPanel::SetSingleton(this);

        using enum Core::AssetType;

        m_AssetIconMap[AUDIO] = ViewerResources::WAVFileIcon;
        m_AssetIconMap[IMAGE] = ViewerResources::PNGFileIcon;
        m_AssetIconMap[CONFIG] = ViewerResources::ConfigFileIcon;
        m_AssetIconMap[ASM] = ViewerResources::ASMFileIcon;
        m_AssetIconMap[BINARY] = ViewerResources::BINARYFileIcon;
        m_AssetIconMap[DISK] = ViewerResources::DISKFileIcon;
        m_AssetIconMap[FONT] = ViewerResources::FontFileIcon;
        m_AssetIconMap[REG_DUMP] = ViewerResources::DumpFileIcon;
        m_AssetIconMap[MEM_DUMP] = ViewerResources::DumpFileIcon;
    }

    void ExplorerPanel::OnProjectChanged() {
        m_Directories.clear();
        m_CurrentItems.Clear();
        m_BaseDirectory = nullptr;
        m_CurrentDirectory = nullptr;
        m_NextDirectory = nullptr;
        m_PreviousDirectory = nullptr;
        SelectionManager::DeselectAll();
        m_BreadCrumbData.clear();

        m_project = AstraProject::CurrentProject();

        UUID baseDirectoryHandle = ProcessDirectory(m_project->rootDirectory, nullptr);
        m_BaseDirectory = m_Directories[baseDirectoryHandle];
        ChangeDirectory(m_BaseDirectory);

        memset(m_SearchBuffer, 0, ContentBrowserItem::MAX_INPUT_BUFFER_LENGTH);
    }

    void ExplorerPanel::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) {
        if (!m_isOpen || !m_project || (UI::Core::Project::CurrentProject() && UI::Core::Project::CurrentProject()->isIsFullScreen())) { return; }
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::Begin(NAME, &m_isOpen, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
            drawPanelContent();
        }
        ImGui::End();
    }

    static float s_Padding = 2.0f;
    static bool s_OpenDeletePopup = false;

    void ExplorerPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        m_IsContentBrowserHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
        m_IsContentBrowserFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        UI::Core::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 8.0f));
        UI::Core::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

        static float w = 250;

        ImGui::BeginChild("content_outliner", ImVec2(w, 0));
        contentOutliner();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::InvisibleButton("##vsplitter", ImVec2(10, -1));
        if (ImGui::IsItemActive())
            w += ImGui::GetIO().MouseDelta.x;
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SameLine();

        ImGui::BeginChild("content_directory");
        contentDirectory();
        ImGui::EndChild();
    }

    void ExplorerPanel::contentDirectory() {
        ENGINE_PROFILE_FUNCTION();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        RenderTopBar();
        ImGui::PopStyleVar();

        ImGui::Separator();

        drawDirectoryContent();

        ImGui::Separator();

        RenderBottomBar();
    }

    void ExplorerPanel::drawDirectoryContent() {
        const auto availSize = ImGui::GetContentRegionAvail();
        const auto contentSize = ImVec2(availSize.x, availSize.y - ImGui::GetTextLineHeightWithSpacing() - 15);

        if (ImGui::BeginChild("Scrolling", contentSize)) {
            UI::Core::ScopedFont font(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::SMALL]);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.35f));

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
            if (ImGui::BeginPopupContextWindow(nullptr, 1)) {
                drawContentPopup();
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar(); // ItemSpacing

            const float paddingForOutline = 2.0f;
            const float scrollBarrOffset = 20.0f + ImGui::GetStyle().ScrollbarSize;
            float panelWidth = ImGui::GetContentRegionAvail().x - scrollBarrOffset;
            float cellSize = m_project->getSettings().ContentBrowserThumbnailSize + s_Padding + paddingForOutline;

            auto columnCount = (int) (panelWidth / cellSize);
            if (columnCount < 1) {
                columnCount = 1;
            }

            {
                const float rowSpacing = 12.0f;
                UI::Core::ScopedStyle spacing2(ImGuiStyleVar_ItemSpacing, ImVec2(paddingForOutline, rowSpacing));
                ImGui::Columns(columnCount, nullptr, false);

                UI::Core::ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);
                UI::Core::ScopedStyle padding2(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
                RenderItems();
            }

            if (ImGui::IsWindowFocused() && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                UpdateInput();
            }

            ImGui::PopStyleColor(2);

            m_questionModal.OnImGuiRender();
        }

        ImGui::EndChild();
    }

    void ExplorerPanel::drawContentPopup() {
        const std::filesystem::path& mainDirectory = m_project->rootDirectory;

        if (ImGui::BeginMenu(I18N::Get("NEW"))) {
            if (ImGui::MenuItem(myFormat(ICON_FA_FOLDER_OPEN " {}", I18N::Get("FOLDER")).c_str())) {
                // NOTE(Peter): For some reason creating new directories through code doesn't trigger a file system change?
                bool created = UI::Core::FileManager::CreateDirectoryA(mainDirectory / m_CurrentDirectory->FilePath / "New Folder");

                if (created) {
                    Refresh();
                    const auto& directoryInfo = GetDirectory(m_CurrentDirectory->FilePath / "New Folder");
                    size_t index = m_CurrentItems.FindItem(directoryInfo->Handle);
                    if (index != ContentBrowserItemList::InvalidItem) {
                        m_CurrentItems[index]->StartRenaming();
                    }
                }
            }

            static const std::pair<const char*, const char*> createFileByType[] = {
                    {"ASM",    "New.asm"},
                    {"CPP",    "New.cpp"},
                    {"Config", "New.yml"},
                    {"Text",   "New.txt"},
                    {"Disk",   "New.dsk"},
            };

            for (const auto& [type, filename]: createFileByType) {
                if (ImGui::MenuItem(myFormat(ICON_FA_FILE " {}", type).c_str())) {
                    const auto& newAsset = CreateAssetInDirectory(filename, m_CurrentDirectory);
                    m_CurrentItems.Items.emplace_back(CreateRef<ContentBrowserAsset>(
                            newAsset,
                            m_AssetIconMap.contains(newAsset.Type) ?
                            m_AssetIconMap[newAsset.Type] : ViewerResources::FileIcon));
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem(I18N::Get("IMPORT"))) {
            std::filesystem::path filepath = UI::Core::FileManager::OpenFolder(I18N::Get("IMPORT_FOLDER"), "");
            if (!filepath.empty()) {
                UI::Core::FileManager::CopyFile2(filepath, mainDirectory / m_CurrentDirectory->FilePath);
                Refresh();
            }
        }

        if (ImGui::MenuItem(I18N::Get("REFRESH"))) {
            Refresh();
        }

        if (ImGui::MenuItem(I18N::Get("COPY"), "Ctrl+C", nullptr,
                            SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) > 0)) {
            m_CopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
        }

        if (ImGui::MenuItem(I18N::Get("PASTE"), "Ctrl+V", nullptr, m_CopiedAssets.SelectionCount() > 0)) {
            PasteCopiedAssets();
        }

        if (ImGui::MenuItem(I18N::Get("DUPLICATE"), "Ctrl+D", nullptr,
                            SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) > 0)) {
            m_CopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
            PasteCopiedAssets();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(I18N::Get("SHOW_EXPLORER"))) {
            UI::Core::FileManager::OpenDirectoryInExplorer(mainDirectory / m_CurrentDirectory->FilePath);
        }
    }

    const UI::Core::AssetMetadata& ExplorerPanel::CreateAssetInDirectory(const std::string& filename, const Ref<DirectoryInfo>& directory) {
        const auto& newAsset = UI::Core::AssetManager::Get().CreateNewAsset(filename, directory->FilePath);
        m_CurrentDirectory->Files.emplace_back(newAsset.Handle);
        return newAsset;
    }

    void ExplorerPanel::contentOutliner() {
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::CollapsingHeader(I18N::Get("CONTENT"), nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            UI::Core::ScopedStyle spacing2(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            UI::Core::ScopedColourStack itemBg(ImGuiCol_Header, IM_COL32_DISABLE,
                                               ImGuiCol_HeaderActive, IM_COL32_DISABLE);

            if (m_BaseDirectory) {
                std::vector<Ref<DirectoryInfo>> directories;
                directories.reserve(m_BaseDirectory->SubDirectories.size());
                for (auto& [name, directory]: m_BaseDirectory->SubDirectories) {
                    directories.emplace_back(directory);
                }

                std::ranges::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b) {
                    return a->FilePath.stem().string() < b->FilePath.stem().string();
                });

                for (const auto& directory: directories) {
                    RenderDirectoryHierarchy(directory);
                }
            }
        }
        // Draw side shadow
        ImRect windowRect = UI::Core::RectExpanded(ImGui::GetCurrentWindow()->Rect(), 0.0f, 10.0f);
        ImGui::PushClipRect(windowRect.Min, windowRect.Max, false);
        UI::Core::DrawShadowInner(ViewerResources::ShadowTexture, 20.0f, windowRect, 1.0f,
                                  windowRect.GetHeight() / 4.0f, false, true, false, false);
        ImGui::PopClipRect();
    }

    static bool s_ActivateSearchWidget = false;

    void ExplorerPanel::RenderTopBar() {
        ImGui::BeginChild("##top_bar", ImVec2(0, 35));

        UI::Core::ShiftCursorY(2);
        const float edgeOffset = 4.0f;

        // Navigation buttons
        {
            UI::Core::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

            auto contenBrowserButton = [](const char* labelId, const Ref<UI::Core::Texture>& icon) {
                const ImU32 buttonCol = UI::Core::Colors::backgroundDark;
                const ImU32 buttonColP = UI::Core::ColourWithMultipliedValue(UI::Core::Colors::backgroundDark, 0.8f);
                UI::Core::ScopedColourStack buttonColours(ImGuiCol_Button, buttonCol,
                                                          ImGuiCol_ButtonHovered, buttonCol,
                                                          ImGuiCol_ButtonActive, buttonColP);

                const float iconSize = 24;
                const float iconPadding = 3.0f;
                const bool clicked = ImGui::Button(labelId, ImVec2(iconSize, iconSize));
                UI::Core::DrawButtonImage(icon, UI::Core::Colors::textDarker,
                                          UI::Core::ColourWithMultipliedValue(UI::Core::Colors::textDarker, 1.2f),
                                          UI::Core::ColourWithMultipliedValue(UI::Core::Colors::textDarker, 0.8f),
                                          UI::Core::RectExpanded(UI::Core::GetItemRect(), -iconPadding, -iconPadding));

                return clicked;
            };

            if (UI::Core::Widgets::IconButton(ICON_FA_BACKWARD)) {
                OnBrowseBack();
            }
            UI::Core::SetTooltip(I18N::Get("PREV_DIRECTORY"));

            ImGui::SameLine();

            if (UI::Core::Widgets::IconButton(ICON_FA_FORWARD)) {
                OnBrowseForward();
            }
            UI::Core::SetTooltip(I18N::Get("NEXT_DIRECTORY"));

            ImGui::SameLine();

            if (UI::Core::Widgets::IconButton(ICON_FA_ROTATE_RIGHT)) {
                Refresh();
            }
            UI::Core::SetTooltip(I18N::Get("REFRESH"));

            ImGui::SameLine(.0f, edgeOffset * 2.0f);
        }

        auto cursopos = ImGui::GetCursorPosX();

        // Search
        {
            UI::Core::ShiftCursorY(1.0f);
            ImGui::SetNextItemWidth(200);

            if (s_ActivateSearchWidget) {
                ImGui::SetKeyboardFocusHere();
                s_ActivateSearchWidget = false;
            }

            if (UI::Core::Widgets::SearchWidget<ContentBrowserItem::MAX_INPUT_BUFFER_LENGTH>("searchBar", m_SearchBuffer)) {
                if (strlen(m_SearchBuffer) == 0) {
                    ChangeDirectory(m_CurrentDirectory);
                } else {
                    m_CurrentItems = Search(m_SearchBuffer, m_CurrentDirectory);
                    SortItemList();
                }
            }
            UI::Core::ShiftCursorY(-2.0f);
        }

        if (m_UpdateNavigationPath) {
            m_BreadCrumbData.clear();

            Ref<DirectoryInfo> current = m_CurrentDirectory;
            while (current && current->Parent != nullptr) {
                m_BreadCrumbData.push_back(current);
                current = current->Parent;
            }

            std::ranges::reverse(m_BreadCrumbData.begin(), m_BreadCrumbData.end());
            m_UpdateNavigationPath = false;
        }

        // Breadcrumbs
        {
            ImGui::SameLine(cursopos + 220);

            UI::Core::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[UI::Core::Fonts::BOLD]);
            UI::Core::ScopedColour textColour(ImGuiCol_Text, UI::Core::Colors::textDarker);

            const std::string& assetsDirectoryName = m_project->rootDirectory.string();
            ImVec2 textSize = ImGui::CalcTextSize(assetsDirectoryName.c_str());
            const float textPadding = ImGui::GetStyle().FramePadding.y;
            if (ImGui::Selectable(assetsDirectoryName.c_str(), false, 0,
                                  ImVec2(textSize.x, textSize.y + textPadding))) {
                ChangeDirectory(m_BaseDirectory);
            }
            UpdateDropArea(m_BaseDirectory);

            for (const auto& directory: m_BreadCrumbData) {
                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();
                std::string directoryName = directory->FilePath.filename().string();
                ImVec2 textSize2 = ImGui::CalcTextSize(directoryName.c_str());
                if (ImGui::Selectable(directoryName.c_str(), false, 0,
                                      ImVec2(textSize2.x, textSize2.y + textPadding))) {
                    ChangeDirectory(directory);
                }

                UpdateDropArea(directory);
            }
        }

        // Settings button
        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        if (UI::Core::Widgets::IconButton(ICON_FA_GEAR)) {
            ImGui::OpenPopup("ContentBrowserSettings");
        }
        UI::Core::SetTooltip(I18N::Get("EXPLORER_SETTINGS"));


        if (UI::Core::BeginPopup("ContentBrowserSettings")) {
            auto& settings = m_project->getSettings();

            ImGui::MenuItem(I18N::Get("SHOW_ASSETS_TYPE"), nullptr,
                            &settings.ContentBrowserShowAssetTypes);
            ImGui::SliderInt("##thumbnail_size", &settings.ContentBrowserThumbnailSize, 96, 512);
            UI::Core::SetTooltip(I18N::Get("THUMBNAIL_SIZE"));

            UI::Core::EndPopup();
        }

        ImGui::EndChild();
    }

    void ExplorerPanel::RenderItems() {
        m_IsAnyItemHovered = false;

        std::scoped_lock<std::mutex> lock(s_LockMutex);
        for (const auto& item: m_CurrentItems) {
            bool isBreaking = renderItemAction(item);
            if (isBreaking) {
                break;
            }
        }

        if (s_OpenDeletePopup) {
            s_OpenDeletePopup = false;
            if (SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) != 0) {
                m_questionModal.OpenQuestion(
                        myFormat(I18N::Get("DELETE_ITEM_QUESTION"), SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser)),
                        [this](bool res) {
                            if (!res) {
                                return;
                            }

                            const auto& selectedItems = SelectionManager::GetSelections(SelectionContext::ContentBrowser);
                            for (UUID handle: selectedItems) {
                                size_t index = m_CurrentItems.FindItem(handle);
                                if (index == ContentBrowserItemList::InvalidItem) {
                                    continue;
                                }

                                m_CurrentItems[index]->Delete();
                                m_CurrentItems.erase(handle);
                            }

                            for (UUID handle: selectedItems) {
                                if (m_Directories.contains(handle))
                                    RemoveDirectory(m_Directories[handle]);
                            }

                            SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
                            Refresh();
                        }
                );
            }
        }
    }

    bool ExplorerPanel::renderItemAction(const Ref<ContentBrowserItem>& item) {
        item->OnRenderBegin();

        CBItemActionResult result = item->OnRender();

        if (result.IsSet(ContentBrowserAction::ClearSelections)) {
            ClearSelections();
        }

        if (result.IsSet(ContentBrowserAction::Selected) &&
            !SelectionManager::IsSelected(SelectionContext::ContentBrowser, item->GetID())) {
            SelectionManager::Select(SelectionContext::ContentBrowser, item->GetID());
        }

        if (result.IsSet(ContentBrowserAction::SelectToHere) &&
            SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) == 2) {
            size_t firstIndex = m_CurrentItems.FindItem(
                    SelectionManager::GetSelection(SelectionContext::ContentBrowser, 0));
            size_t lastIndex = m_CurrentItems.FindItem(item->GetID());

            if (firstIndex > lastIndex) {
                size_t temp = firstIndex;
                firstIndex = lastIndex;
                lastIndex = temp;
            }

            for (size_t i = firstIndex; i <= lastIndex; i++) {
                SelectionManager::Select(SelectionContext::ContentBrowser, m_CurrentItems[i]->GetID());
            }
        }

        if (result.IsSet(ContentBrowserAction::StartRenaming)) {
            item->StartRenaming();
        }

        if (result.IsSet(ContentBrowserAction::Copy)) {
            m_CopiedAssets.Select(item->GetID());
        }

        if (result.IsSet(ContentBrowserAction::Reload)) {
            buildItem(item->GetID());
        }

        if (result.IsSet(ContentBrowserAction::OpenDeleteDialogue)) {
            s_OpenDeletePopup = true;
        }

        if (result.IsSet(ContentBrowserAction::ShowInExplorer)) {
            if (item->GetType() == ContentBrowserItem::ItemType::Directory) {
                UI::Core::FileManager::ShowFileInExplorer(m_project->rootDirectory / m_CurrentDirectory->FilePath / item->GetName());
            } else {
                UI::Core::FileManager::ShowFileInExplorer(UI::Core::AssetMetadata::GetFileSystemPath(UI::Core::AssetManager::Get().GetMetadata(item->GetID())));
            }
        }

        if (result.IsSet(ContentBrowserAction::OpenExternal)) {
            if (item->GetType() == ContentBrowserItem::ItemType::Directory) {
                UI::Core::FileManager::OpenExternally(m_project->rootDirectory / m_CurrentDirectory->FilePath / item->GetName());
            } else {
                UI::Core::FileManager::OpenExternally(UI::Core::AssetMetadata::GetFileSystemPath(UI::Core::AssetManager::Get().GetMetadata(item->GetID())));
            }
        }

        if (result.IsSet(ContentBrowserAction::Hovered)) {
            m_IsAnyItemHovered = true;
        }

        item->OnRenderEnd();

        if (result.IsSet(ContentBrowserAction::Duplicate)) {
            m_CopiedAssets.Select(item->GetID());
            PasteCopiedAssets();
            return true;
        }

        if (result.IsSet(ContentBrowserAction::Renamed)) {
            SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
            RefreshWithoutLock();
            SortItemList();

            if (item == nullptr) {
                return true;
            }

            SelectionManager::Select(SelectionContext::ContentBrowser, item->GetID());
            return true;
        }

        if (result.IsSet(ContentBrowserAction::NavigateToThis)) {
            SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
            auto newDir = dynamic_cast<ContentBrowserDirectory*>(item.get())->GetDirectoryInfo();
            ChangeDirectory(newDir);
            return true;
        }

        if (result.IsSet(ContentBrowserAction::Refresh)) {
            RefreshWithoutLock();
            return true;
        }

        return false;
    }

    void ExplorerPanel::RenderBottomBar() {
        ENGINE_PROFILE_FUNCTION();

        UI::Core::ScopedStyle childBorderSize(ImGuiStyleVar_ChildBorderSize, 0);
        UI::Core::ScopedStyle frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0);
        UI::Core::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        UI::Core::ScopedStyle framePadding(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (ImGui::BeginChild("##bottom_bar")) {
            size_t selectionCount = SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser);
            if (selectionCount == 1) {
                UUID firstSelection = SelectionManager::GetSelection(SelectionContext::ContentBrowser, 0);

                std::string filepath;
                if (m_Directories.contains(firstSelection)) {
                    filepath = "Assets/" + m_Directories[firstSelection]->FilePath.string();
                } else if (const auto& assetMetadata = UI::Core::AssetManager::Get().GetMetadata(firstSelection); assetMetadata.isValid()) {
                    filepath = "Assets/" + assetMetadata.FilePath.string();
                }

                std::ranges::replace(filepath.begin(), filepath.end(), '\\', '/');
                ImGui::TextUnformatted(filepath.c_str());
            } else if (selectionCount > 1) {
                ImGui::Text(I18N::Get("ITEM_SELECTED_NB"), selectionCount);
            }
        }

        ImGui::EndChild();
    }

    void ExplorerPanel::RenderDirectoryHierarchy(const Ref<DirectoryInfo>& directory) {
        std::string name = directory->FilePath.filename().string();
        std::string id = name + "_TreeNode";
        bool previousState = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(id.c_str()));

        // ImGui item height hack
        auto* window = ImGui::GetCurrentWindow();
        window->DC.CurrLineSize.y = 20.0f;
        window->DC.CurrLineTextBaseOffset = 3.0f;
        //---------------------------------------------

        const ImRect itemRect = {
                window->WorkRect.Min.x, window->DC.CursorPos.y,
                window->WorkRect.Max.x, window->DC.CursorPos.y + window->DC.CurrLineSize.y
        };

        const bool isItemClicked = [&itemRect, &id] {
            if (ImGui::ItemHoverable(itemRect, ImGui::GetID(id.c_str()), ImGuiItemFlags_None)) {
                return ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
            }
            return false;
        }();

        const bool isWindowFocused = ImGui::IsWindowFocused();


        auto fillWithColour = [&](const ImColor& colour) {
            const ImU32 bgColour = ImGui::ColorConvertFloat4ToU32(colour);
            ImGui::GetWindowDrawList()->AddRectFilled(itemRect.Min, itemRect.Max, bgColour);
        };

        const bool isActiveDirectory = directory->Handle == m_CurrentDirectory->Handle;

        ImGuiTreeNodeFlags flags =
                (isActiveDirectory ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanFullWidth;

        // Fill background
        //----------------
        if (isActiveDirectory || isItemClicked) {
            if (isWindowFocused) {
                fillWithColour(UI::Core::Colors::selection);
            } else {
                const ImColor col = UI::Core::ColourWithMultipliedValue(UI::Core::Colors::selection, 0.8f);
                fillWithColour(UI::Core::ColourWithMultipliedSaturation(col, 0.7f));
            }

            ImGui::PushStyleColor(ImGuiCol_Text, UI::Core::Colors::backgroundDark);
        } else if (checkIfAnyDescendantSelected(directory)) {
            fillWithColour(UI::Core::Colors::selectionMuted);
        }

        // Tree Node
        //----------

        bool open = UI::Core::TreeNode(id, name, flags, ViewerResources::FolderIcon);

        if (isActiveDirectory || isItemClicked) {
            ImGui::PopStyleColor();
        }

        // Fixing slight overlap
        UI::Core::ShiftCursorY(3.0f);

        // Create Menu
        //------------
        drawContextMenu(directory);

        // Draw children
        //--------------

        if (open) {
            std::vector<Ref<DirectoryInfo>>
                    directories;
            directories.reserve(directory->SubDirectories.size());
            for (auto& [handle, dir]: directory->SubDirectories) {
                directories.emplace_back(dir);
            }

            std::ranges::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b) {
                return a->FilePath.stem().string() < b->FilePath.stem().string();
            });

            for (const auto& child: directories) {
                RenderDirectoryHierarchy(child);
            }
        }

        UpdateDropArea(directory);

        if (open != previousState && !isActiveDirectory && !ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.01f)) {
            ChangeDirectory(directory);
        }

        if (open) {
            ImGui::TreePop();
        }

    }

    void ExplorerPanel::drawContextMenu(const Ref<DirectoryInfo>& directory) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
        if (ImGui::BeginPopupContextItem()) {
            const std::filesystem::path& mainDirectory = m_project->rootDirectory;
            if (ImGui::BeginMenu(I18N::Get("NEW"))) {
                if (ImGui::MenuItem(I18N::Get("FOLDER"))) {
                    bool needRefresh = UI::Core::FileManager::CreateDirectoryA(mainDirectory / directory->FilePath / "New Folder");
                    if (needRefresh) {
                        Refresh();
                    }
                }

                static const std::pair<const char*, const char*> createFiles[] = {
                        {"ASM",    "New.asm"},
                        {"CPP",    "New.cpp"},
                        {"Config", "New.yml"},
                        {"Text",   "New.txt"}
                };

                for (const auto& [fileType, fileName]: createFiles) {
                    if (ImGui::MenuItem(fileType)) {
                        CreateAssetInDirectory(fileName, directory);
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(I18N::Get("SHOW_EXPLORER"))) {
                UI::Core::FileManager::OpenDirectoryInExplorer(mainDirectory / directory->FilePath);
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(); // ItemSpacing

    }

// Fill with light selection colour if any of the child entities selected
    bool ExplorerPanel::checkIfAnyDescendantSelected(const Ref<DirectoryInfo>& directory) {
        if (directory->Handle == m_CurrentDirectory->Handle) {
            return true;
        }

        if (!directory->SubDirectories.empty()) {
            for (const auto& [childHandle, childDir]: directory->SubDirectories) {
                if (checkIfAnyDescendantSelected(childDir)) {
                    return true;
                }
            }
        }

        return false;
    }

    void ExplorerPanel::OnBrowseBack() {
        m_NextDirectory = m_CurrentDirectory;
        m_PreviousDirectory = m_CurrentDirectory->Parent;
        ChangeDirectory(m_PreviousDirectory);
    }

    void ExplorerPanel::OnBrowseForward() {
        ChangeDirectory(m_NextDirectory);
    }

    void ExplorerPanel::ChangeDirectory(const Ref<DirectoryInfo>& directory) {
        if (!directory) {
            return;
        }

        m_UpdateNavigationPath = true;

        m_CurrentItems.Items.clear();
        SelectionManager::DeselectAll(SelectionContext::ContentBrowser);

        if (strlen(m_SearchBuffer) == 0) {
            for (auto& [subdirHandle, subdir]: directory->SubDirectories) {
                m_CurrentItems.Items.push_back(CreateRef<ContentBrowserDirectory>(subdir));
            }

            std::vector<UUID> invalidAssets;
            for (auto assetHandle: directory->Files) {
                UI::Core::AssetMetadata metadata = UI::Core::AssetManager::Get().GetMetadata(assetHandle);
                if (metadata.isValid()) {
                    m_CurrentItems.Items.push_back(CreateRef<ContentBrowserAsset>(
                            metadata,
                            m_AssetIconMap.contains(metadata.Type) ?
                            m_AssetIconMap[metadata.Type] : ViewerResources::FileIcon
                    ));
                }
            }
        } else {
            m_CurrentItems = Search(m_SearchBuffer, directory);
        }

        SortItemList();

        m_PreviousDirectory = directory;
        m_CurrentDirectory = directory;
    }

    void ExplorerPanel::RemoveDirectory(const Ref<DirectoryInfo>& directory, bool removeFromParent) {
        if (directory->Parent && removeFromParent) {
            auto& childList = directory->Parent->SubDirectories;
            childList.erase(childList.find(directory->Handle));
        }

        for (const auto& [handle, subdir]: directory->SubDirectories) {
            RemoveDirectory(subdir, false);
        }

        directory->SubDirectories.clear();
        directory->Files.clear();

        m_Directories.erase(m_Directories.find(directory->Handle));
    }

    void ExplorerPanel::UpdateInput() const {
        if (!m_IsContentBrowserHovered) {
            return;
        }

        if (!m_IsAnyItemHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ClearSelections();
        }
    }

    void ExplorerPanel::ClearSelections() const {
        for (auto& item: m_CurrentItems) {
            SelectionManager::Deselect(SelectionContext::ContentBrowser, item->GetID());

            if (item->IsRenaming()) {
                item->StopRenaming();
            }
        }
    }

    void ExplorerPanel::SortItemList() {
        std::ranges::sort(m_CurrentItems.begin(), m_CurrentItems.end(),
                          [](const auto& item1, const auto& item2) {
                              if (item1->GetType() == item2->GetType()) {
                                  return Utils::ToLower(item1->GetName()) < Utils::ToLower(item2->GetName());
                              }

                              return (uint16_t) item1->GetType() < (uint16_t) item2->GetType();
                          });
    }

    ContentBrowserItemList ExplorerPanel::Search(const std::string& query, const Ref<DirectoryInfo>& directoryInfo) {
        ContentBrowserItemList results;
        std::string queryLowerCase = Utils::ToLower(query);

        for (auto& [handle, subdir]: directoryInfo->SubDirectories) {
            std::string subdirName = subdir->FilePath.filename().string();
            if (subdirName.find(queryLowerCase) != std::string::npos) {
                results.Items.push_back(CreateRef<ContentBrowserDirectory>(subdir));
            }

            ContentBrowserItemList list = Search(query, subdir);
            results.Items.insert(results.Items.end(), list.Items.begin(), list.Items.end());
        }

        for (const auto& assetHandle: directoryInfo->Files) {
            auto& asset = UI::Core::AssetManager::Get().GetMetadata(assetHandle);
            if (!asset.isValid()) { continue; }

            std::string filename = Utils::ToLower(asset.FilePath.filename().string());

            if (filename.find(queryLowerCase) != std::string::npos) {
                results.Items.push_back(CreateRef<ContentBrowserAsset>(
                        asset,
                        m_AssetIconMap.contains(asset.Type) ?
                        m_AssetIconMap[asset.Type] : ViewerResources::FileIcon
                ));
            }
        }

        return results;
    }

    void ExplorerPanel::UpdateDropArea(const Ref<DirectoryInfo>& target) {
        if (target->Handle != m_CurrentDirectory->Handle && ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload");

            if (payload) {
                uint32_t count = payload->DataSize / sizeof(UUID);

                for (uint32_t i = 0; i < count; i++) {
                    UUID assetHandle = *(((UUID*) payload->Data) + i);
                    size_t index = m_CurrentItems.FindItem(assetHandle);
                    if (index != ContentBrowserItemList::InvalidItem) {
                        m_CurrentItems[index]->Move(target->FilePath);
                        m_CurrentItems.erase(assetHandle);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
    }

    Ref<DirectoryInfo> ExplorerPanel::GetDirectory(const std::filesystem::path& filepath) const {
        if (filepath.string().empty() || filepath.string() == ".") {
            return m_BaseDirectory;
        }

        for (const auto& [handle, directory]: m_Directories) {
            if (directory->FilePath == filepath) {
                return directory;
            }
        }

        return nullptr;
    }

    void ExplorerPanel::Refresh() {
        std::scoped_lock lock{s_LockMutex};
        RefreshWithoutLock();
    }

    void ExplorerPanel::RefreshWithoutLock() {
        m_CurrentItems.Clear();
        m_Directories.clear();

        Ref<DirectoryInfo> currentDirectory = m_CurrentDirectory;
        UUID baseDirectoryHandle = ProcessDirectory(m_project->rootDirectory, nullptr);
        m_BaseDirectory = m_Directories[baseDirectoryHandle];
        m_CurrentDirectory = GetDirectory(currentDirectory->FilePath);

        if (!m_CurrentDirectory) {
            m_CurrentDirectory = m_BaseDirectory;
        } // Our current directory was removed

        ChangeDirectory(m_CurrentDirectory);
    }

    UUID ExplorerPanel::ProcessDirectory(const std::filesystem::path& directoryPath, const Ref<DirectoryInfo>& parent) {
        const auto& directory = GetDirectory(directoryPath);
        if (directory) {
            return directory->Handle;
        }

        Ref<DirectoryInfo> directoryInfo = CreateRef<DirectoryInfo>();
        directoryInfo->Handle = UUIDGen::New();
        directoryInfo->Parent = parent;

        const std::filesystem::path& mainDirectory = m_project->rootDirectory;

        if (directoryPath == mainDirectory) {
            directoryInfo->FilePath = "";
        } else {
            directoryInfo->FilePath = std::filesystem::relative(directoryPath, mainDirectory);
        }

        for (const auto& entry: std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_directory()) {
                UUID subdirHandle = ProcessDirectory(entry.path(), directoryInfo);
                directoryInfo->SubDirectories[subdirHandle] = m_Directories[subdirHandle];
            } else {
                auto metadata = UI::Core::AssetManager::Get().GetOrCreateMetadata(std::filesystem::relative(entry.path(), mainDirectory));
                if (!metadata.isValid()) {
                    continue;
                }

                directoryInfo->Files.push_back(metadata.Handle);
            }
        }

        m_Directories[directoryInfo->Handle] = directoryInfo;
        return directoryInfo->Handle;
    }

    void ExplorerPanel::OnEvent(UI::Core::AEvent& pEvent) {
        switch (pEvent.GetEventType()) {
            case UI::Core::EventType::ProjectLoaded:
                OnProjectChanged();
                break;
            case UI::Core::EventType::WindowFocus: {
                const auto& windowFocusEvt = dynamic_cast<const UI::Core::WindowFocusEvent&>(pEvent);
                if (windowFocusEvt.isFocus()) {
                    LOG_DEBUG("ExplorerPanel: refresh on focus");
                    Refresh();
                }
                break;
            }
            case UI::Core::EventType::WindowDrop: {
                const auto& dropEvt = dynamic_cast<const UI::Core::WindowDropEvent&>(pEvent);
                for (const auto& path: dropEvt.GetPaths()) {
                    UI::Core::FileManager::CopyFile2(path, m_project->rootDirectory / m_CurrentDirectory->FilePath);
                    LOG_DEBUG("Copying path {} to directory {}", path, m_project->rootDirectory / m_CurrentDirectory->FilePath);
                }
                Refresh();
                pEvent.Handled = true;
                break;
            }
            default:
                break;
        }
    }

    void ExplorerPanel::PasteCopiedAssets() {
        if (m_CopiedAssets.SelectionCount() == 0) {
            return;
        }

        for (UUID copiedAsset: m_CopiedAssets) {
            size_t assetIndex = m_CurrentItems.FindItem(copiedAsset);

            if (assetIndex == ContentBrowserItemList::InvalidItem) {
                continue;
            }

            const auto& item = m_CurrentItems[assetIndex];
            auto originalFilePath = m_project->rootDirectory;

            item->PasteCopiedAssets(originalFilePath);
        }

        Refresh();
        SelectionManager::DeselectAll();
        m_CopiedAssets.Clear();
    }

    void ExplorerPanel::buildItem(UUID id) const {
        LOG_DEBUG("[ExplorerPanel] Build item {}", id);

        auto& metadata = UI::Core::AssetManager::Get().GetMetadata(id);
        if (!metadata.isValid()) {
            LOG_ERROR("[ExplorerPanel] Trying to build invalid asset {}", id);
            UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("TRY_BUILD_ASSET"), id));
            return;
        }

        switch (metadata.Type) {
            using
            enum UI::Core::AssetType;
            case MEM_DUMP: {
                /*std::ifstream file(UI::Core::AssetMetadata::GetFileSystemPath(metadata));
                if (file.good()) {
                    CPU::Core::DevicesManager::Get().getRamMemory().loadFromStream(file);
                    file.close();
                    UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(
                            AstraMessage::New2(AstraMessageType::Info, "Asset {} reloaded in ram at addr 0", metadata.FilePath));
                } else {
                    UI::Core::Events::GetLink().OnEvent<UI::Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Error, "Can not open file {}", metadata.FilePath));
                }*/
                break;
            }
            case CPP: {
                //CPU::Core::CoreLibManager::GetLink().recompileByUUID(metadata.Handle);
                break;
            }
            case ASM: {
                auto engine = AstraProject::CurrentProject()->getCurrentEngine();
                if (engine) {
                    auto outBinPath = metadata.FilePath.parent_path() / metadata.FilePath.stem();
                    outBinPath += ".bin";
                    //CPU::Core::AsmCompiler::GetLink().asmCompile(metadata.FilePath, outBinPath, engine);
                } else {
                    LOG_WARN("[ExplorerPanel] No engine selected for build, skipped");
                    UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Warning, I18N::Get("NO_ENGINE_BUILD")));
                }
                break;
            }
            default:
                break;
        }
    }

}
