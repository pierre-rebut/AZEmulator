#pragma once

#include <unordered_map>
#include <vector>
#include "Commons/utils/UUID.h"

namespace Astra::UI::App {

    enum class SelectionContext
    {
        Global = 0, Scene, ContentBrowser, PrefabEditor
    };

    using SelectionContextType = std::unordered_map<SelectionContext, std::vector<UUID>>;

    class SelectionManager
    {
    public:
        static void Select(SelectionContext context, UUID selectionID);
        static bool IsSelected(UUID selectionID);
        static bool IsSelected(SelectionContext context, UUID selectionID);
        static void Deselect(UUID selectionID);
        static void Deselect(SelectionContext context, UUID selectionID);
        static void DeselectAll();
        static void DeselectAll(SelectionContext context);
        static UUID GetSelection(SelectionContext context, size_t index);

        static size_t GetSelectionCount(SelectionContext contextID);

        inline static const std::vector<UUID>& GetSelections(SelectionContext context) { return s_Contexts[context]; }

    private:
        inline static SelectionContextType s_Contexts;
    };

}
