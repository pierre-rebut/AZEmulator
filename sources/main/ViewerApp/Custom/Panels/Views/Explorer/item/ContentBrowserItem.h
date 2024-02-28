#pragma once

#include "EngineLib/data/Base.h"
#include "Commons/utils/UUID.h"
#include "ViewerApp/CoreLib/Resources/Texture.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"

#include <filesystem>
#include <map>
#include <vector>

namespace Astra::UI::App {

    enum class ContentBrowserAction
    {
        None = 0,
        Refresh = BIT(0),
        ClearSelections = BIT(1),
        Selected = BIT(2),
        Hovered = BIT(3),
        Renamed = BIT(4),
        NavigateToThis = BIT(5),
        OpenDeleteDialogue = BIT(6),
        SelectToHere = BIT(7),
        Moved = BIT(8),
        ShowInExplorer = BIT(9),
        OpenExternal = BIT(10),
        Reload = BIT(11),
        Copy = BIT(12),
        Duplicate = BIT(13),
        StartRenaming = BIT(14)
    };

    struct CBItemActionResult
    {
        uint16_t Field = 0;

        void Set(ContentBrowserAction flag, bool value) {
            if (value)
                Field |= (uint16_t) flag;
            else
                Field &= ~(uint16_t) flag;
        }

        bool IsSet(ContentBrowserAction flag) const { return (uint16_t) flag & Field; }
    };

    class ContentBrowserItem
    {
    public:
        static constexpr const int MAX_INPUT_BUFFER_LENGTH = 128;

        enum class ItemType : uint16_t
        {
            Directory, Asset
        };
        ContentBrowserItem(ItemType type, UUID id, const std::string& name, const Ref<UI::Core::Texture>& icon);
        virtual ~ContentBrowserItem() = default;

        void OnRenderBegin() const;
        CBItemActionResult OnRender();
        void OnRenderEnd() const;

        virtual void Delete() { /* do nothing */ }

        virtual bool Move(const std::filesystem::path& destination) { return false; }

        UUID GetID() const { return m_ID; }

        ItemType GetType() const { return m_Type; }

        const std::string& GetName() const { return m_Name; }

        const Ref<UI::Core::Texture>& GetIcon() const { return m_Icon; }

        virtual void Activate(CBItemActionResult& actionResult) { /* do nothing */ }
        virtual void OnDragDropItem() const { /* do nothing */ }
        virtual void PasteCopiedAssets(const std::filesystem::path& originalFilePath) = 0;

        void StartRenaming();
        void StopRenaming();

        bool IsRenaming() const { return m_IsRenaming; }

        void Rename(const std::string& newName);

    private:
        virtual void OnRenamed(const std::string& newName) { m_Name = newName; }

        virtual void RenderCustomContextItems() { /* do nothing */ }

        virtual void UpdateDrop(CBItemActionResult& actionResult) { /* do nothing */ }

        void OnContextMenuOpen(CBItemActionResult& actionResult);

    protected:
        ItemType m_Type;
        UUID m_ID;
        std::string m_Name;
        const Ref<UI::Core::Texture>& m_Icon;

        bool m_IsRenaming = false;
        bool m_IsDragging = false;
        void dragDropSourceItems(const std::vector<UUID>& selectionStack) const;
    };

}
