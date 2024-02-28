
#include "ContentBrowserItem.h"

#include "imgui_internal.h"

#include "ViewerApp/CoreLib/Utils.h"
#include "ViewerApp/Custom/Utils/SelectionManager.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "Commons/utils/Utils.h"
#include "ViewerApp/Custom/Panels/Views/ExplorerPanel.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {

    static char s_RenameBuffer[ContentBrowserItem::MAX_INPUT_BUFFER_LENGTH];

    ContentBrowserItem::ContentBrowserItem(ItemType type, UUID id, const std::string& name, const Ref<UI::Core::Texture>& icon)
            : m_Type(type), m_ID(id), m_Icon(icon) {
        m_Name = Utils::StrTruncate(name, 30);
    }

    void ContentBrowserItem::OnRenderBegin() const {
        ImGui::PushID(&m_ID);
        ImGui::BeginGroup();
    }

    CBItemActionResult ContentBrowserItem::OnRender() {
        CBItemActionResult result;

        const ProjectSettings& settings = AstraProject::CurrentProject()->getSettings();
        const float thumbnailSize = (float) settings.ContentBrowserThumbnailSize;
        const bool displayAssetType = settings.ContentBrowserShowAssetTypes;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        const float edgeOffset = 4.0f;

        const float textLineHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f + edgeOffset * 2.0f;
        const float infoPanelHeight = std::max(displayAssetType ? thumbnailSize * 0.5f : textLineHeight,
                                               textLineHeight);

        const ImVec2 topLeft = ImGui::GetCursorScreenPos();
        const ImVec2 thumbBottomRight = {topLeft.x + thumbnailSize, topLeft.y + thumbnailSize};
        const ImVec2 infoTopLeft = {topLeft.x, topLeft.y + thumbnailSize};
        const ImVec2 bottomRight = {topLeft.x + thumbnailSize, topLeft.y + thumbnailSize + infoPanelHeight};

        auto drawShadow = [](const ImVec2& topLeft, const ImVec2& bottomRight, bool directory) {
            auto* drawList = ImGui::GetWindowDrawList();
            const ImRect itemRect = UI::Core::RectOffset(ImRect(topLeft, bottomRight), 1.0f, 1.0f);
            drawList->AddRect(itemRect.Min, itemRect.Max, UI::Core::Colors::propertyField, 6.0f,
                              directory ? 0 : ImDrawFlags_RoundCornersBottom, 2.0f);
        };

        const bool isSelected = SelectionManager::IsSelected(SelectionContext::ContentBrowser, m_ID);

        // Fill background
        //----------------

        if (m_Type != ItemType::Directory) {
            auto* drawList = ImGui::GetWindowDrawList();

            // Draw shadow
            drawShadow(topLeft, bottomRight, false);

            // Draw background
            drawList->AddRectFilled(topLeft, thumbBottomRight, UI::Core::Colors::backgroundDark);
            drawList->AddRectFilled(infoTopLeft, bottomRight, UI::Core::Colors::groupHeader, 6.0f,
                                    ImDrawFlags_RoundCornersBottom);
        } else if (ImGui::ItemHoverable(ImRect(topLeft, bottomRight), ImGui::GetID(&m_ID), ImGuiItemFlags_None) || isSelected) {
            // If hovered or selected directory

            // Draw shadow
            drawShadow(topLeft, bottomRight, true);

            auto* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(topLeft, bottomRight, UI::Core::Colors::groupHeader, 6.0f);
        }

        auto itemRect = ImRect(topLeft, bottomRight);

        // Thumbnail
        //==========
        // TODO: replace with actual Asset Thumbnail interface

        ImGui::InvisibleButton("##thumbnailButton", ImVec2{thumbnailSize, thumbnailSize});
        UI::Core::DrawButtonImage(m_Icon, IM_COL32(255, 255, 255, 225),
                            IM_COL32(255, 255, 255, 255),
                            IM_COL32(255, 255, 255, 255),
                            UI::Core::RectExpanded(UI::Core::GetItemRect(), -6.0f, -6.0f));

        // Info Panel
        //-----------

        auto renamingWidget = [&] {
            ImGui::SetKeyboardFocusHere();
            ImGui::InputText("##rename", s_RenameBuffer, MAX_INPUT_BUFFER_LENGTH);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
                Rename(s_RenameBuffer);
                m_IsRenaming = false;
                result.Set(ContentBrowserAction::Renamed, true);
            }
        };

        auto pos = ImGui::GetCursorPosY();
        ImGui::ItemSize(ImVec2(thumbnailSize - edgeOffset * 3.0f, infoPanelHeight - edgeOffset));
        ImGui::SetCursorPosY(pos);
        UI::Core::ShiftCursor(edgeOffset, edgeOffset);
        if (m_Type == ItemType::Directory) {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (thumbnailSize - edgeOffset * 3.0f));
            const float textWidth = std::min(ImGui::CalcTextSize(m_Name.c_str()).x, thumbnailSize);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((thumbnailSize - edgeOffset * 3.0f) / 2) - (textWidth / 2));
            if (m_IsRenaming) {
                ImGui::SetNextItemWidth(thumbnailSize - edgeOffset * 3.0f);
                renamingWidget();
            } else {
                ImGui::SetNextItemWidth(textWidth);
                ImGui::Text(m_Name.c_str());
            }
            ImGui::PopTextWrapPos();

        } else {
            auto pos2 = ImGui::GetCursorPosY();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (thumbnailSize - edgeOffset * 2.0f));
            if (m_IsRenaming) {
                ImGui::SetNextItemWidth(thumbnailSize - edgeOffset * 3.0f);
                renamingWidget();
            } else {
                ImGui::Text(m_Name.c_str());
            }
            ImGui::PopTextWrapPos();

            if (displayAssetType) {
                const UI::Core::AssetMetadata& metadata = UI::Core:: AssetManager::Get().GetMetadata(m_ID);
                if (metadata.isValid()) {
                    const std::string& assetType = Utils::ToUpper(UI::Core::AssetMetadata::AssetTypeToString(metadata.Type));

                    ImGui::SetCursorPos(ImVec2(
                            ImGui::GetCursorPosX() + thumbnailSize - ImGui::CalcTextSize(assetType.c_str()).x - 5,
                            pos2 + infoPanelHeight - edgeOffset - ImGui::GetTextLineHeightWithSpacing() - 2)
                    );

                    UI::Core::ScopedColour textColour(ImGuiCol_Text, UI::Core::Colors::textDarker);
                    ImGui::TextUnformatted(assetType.c_str());
                }
            }
        }
        UI::Core::ShiftCursor(-edgeOffset, -edgeOffset);

        if (!m_IsRenaming && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2)) && isSelected) {
            StartRenaming();
        }

        ImGui::PopStyleVar(); // ItemSpacing

        // End of the Item Group
        //======================
        ImGui::EndGroup();

        // Draw outline
        //-------------

        if (isSelected || ImGui::IsItemHovered()) {
            auto* drawList = ImGui::GetWindowDrawList();

            if (isSelected) {
                const bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsItemHovered();
                auto colTransition = UI::Core::ColourWithMultipliedValue(UI::Core::Colors::selection, 0.8f);

                drawList->AddRect(itemRect.Min, itemRect.Max,
                                  mouseDown ? colTransition : UI::Core::Colors::selection, 6.0f,
                                  m_Type == ItemType::Directory ? 0 : ImDrawFlags_RoundCornersBottom, 1.0f);
            } else // isHovered
            {
                if (m_Type != ItemType::Directory) {
                    drawList->AddRect(itemRect.Min, itemRect.Max,
                                      UI::Core::Colors::muted, 6.0f,
                                      ImDrawFlags_RoundCornersBottom, 1.0f);
                }
            }
        }

        // Mouse Events handling
        //======================

        UpdateDrop(result);

        bool dragging = ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID);
        if (dragging) {
            m_IsDragging = true;

            const auto& selectionStack = SelectionManager::GetSelections(SelectionContext::ContentBrowser);
            if (!SelectionManager::IsSelected(SelectionContext::ContentBrowser, m_ID)) {
                result.Set(ContentBrowserAction::ClearSelections, true);
            }

            if (!selectionStack.empty()) {
                dragDropSourceItems(selectionStack);
            }

            result.Set(ContentBrowserAction::Selected, true);
            ImGui::EndDragDropSource();
        }

        if (ImGui::IsItemHovered()) {
            result.Set(ContentBrowserAction::Hovered, true);

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                Activate(result);
            } else {
                bool action = SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) > 1
                              ? ImGui::IsMouseReleased(ImGuiMouseButton_Left) : ImGui::IsMouseClicked(
                                ImGuiMouseButton_Left);
                bool skipBecauseDragging =
                        m_IsDragging && SelectionManager::IsSelected(SelectionContext::ContentBrowser, m_ID);
                if (action && !skipBecauseDragging) {
                    result.Set(ContentBrowserAction::Selected, true);

                    if (!ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) &&
                        !ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftShift))) {
                        result.Set(ContentBrowserAction::ClearSelections, true);
                    }

                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftShift))) {
                        result.Set(ContentBrowserAction::SelectToHere, true);
                    }
                }
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
        if (ImGui::BeginPopupContextItem("CBItemContextMenu")) {
            result.Set(ContentBrowserAction::Selected, true);

            if (!ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) &&
                !ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftShift))) {
                result.Set(ContentBrowserAction::ClearSelections, true);
            }

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftShift))) {
                result.Set(ContentBrowserAction::SelectToHere, true);
            }

            OnContextMenuOpen(result);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        m_IsDragging = dragging;

        return result;
    }

    void ContentBrowserItem::dragDropSourceItems(const std::vector<UUID>& selectionStack) const {
        auto& currentItems = ExplorerPanel::Get().GetCurrentItems();

        for (const auto& selectedItemHandles: selectionStack) {
            size_t index = currentItems.FindItem(selectedItemHandles);
            if (index == ContentBrowserItemList::InvalidItem) {
                continue;
            }

            const auto& item = currentItems[index];
            ImGui::Image(item->GetIcon()->textureId(), ImVec2(20, 20));
            ImGui::SameLine();
            const auto& name = item->GetName();
            ImGui::TextUnformatted(name.c_str());
        }

        if (selectionStack.size() > 1) {
            ImGui::SetDragDropPayload("assets_payload", selectionStack.data(), sizeof(UUID) * selectionStack.size());
        } else {
            OnDragDropItem();
        }
    }

    void ContentBrowserItem::OnRenderEnd() const {
        ImGui::PopID();
        ImGui::NextColumn();
    }

    void ContentBrowserItem::StartRenaming() {
        if (m_IsRenaming) {
            return;
        }

        memset(s_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
        memcpy(s_RenameBuffer, m_Name.c_str(), m_Name.size());
        m_IsRenaming = true;
    }

    void ContentBrowserItem::StopRenaming() {
        m_IsRenaming = false;
        memset(s_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
    }

    void ContentBrowserItem::Rename(const std::string& newName) {
        OnRenamed(newName);
    }

    void ContentBrowserItem::OnContextMenuOpen(CBItemActionResult& actionResult) {

        if (ImGui::MenuItem(I18N::Get("RELOAD"))) {
            actionResult.Set(ContentBrowserAction::Reload, true);
        }

        if (ImGui::MenuItem(I18N::Get("RENAME"))) {
            actionResult.Set(ContentBrowserAction::StartRenaming, true);
        }

        if (ImGui::MenuItem(I18N::Get("COPY"))) {
            actionResult.Set(ContentBrowserAction::Copy, true);
        }

        if (ImGui::MenuItem(I18N::Get("DUPLICATE"))) {
            actionResult.Set(ContentBrowserAction::Duplicate, true);
        }

        if (ImGui::MenuItem(I18N::Get("DELETE"))) {
            actionResult.Set(ContentBrowserAction::OpenDeleteDialogue, true);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(I18N::Get("SHOW_EXPLORER"))) {
            actionResult.Set(ContentBrowserAction::ShowInExplorer, true);
        }
        if (ImGui::MenuItem(I18N::Get("OPEN_EXTERNAL"))) {
            actionResult.Set(ContentBrowserAction::OpenExternal, true);
        }

        RenderCustomContextItems();
    }

}
