
#include <algorithm>
#include "SelectionManager.h"
#include "Commons/AstraException.h"

namespace Astra::UI::App {

    void SelectionManager::Select(SelectionContext contextID, UUID selectionID) {
        auto& contextSelections = s_Contexts[contextID];
        if (std::ranges::find(contextSelections.begin(), contextSelections.end(), selectionID) != contextSelections.end())
            return;

        // TODO: Maybe verify if the selectionID is already selected in another context?

        contextSelections.push_back(selectionID);
        //Application::GetLink().DispatchEvent<SelectionChangedEvent>(contextID, selectionID, true);
    }

    bool SelectionManager::IsSelected(UUID selectionID) {
        return std::ranges::any_of(s_Contexts, [selectionID](const SelectionContextType::value_type& e) {
            return std::ranges::find(e.second.begin(), e.second.end(), selectionID) != e.second.end();
        });
    }

    bool SelectionManager::IsSelected(SelectionContext contextID, UUID selectionID) {
        const auto& contextSelections = s_Contexts[contextID];
        return std::ranges::find(contextSelections.begin(), contextSelections.end(), selectionID) != contextSelections.end();
    }

    void SelectionManager::Deselect(UUID selectionID) {
        for (auto& [contextID, contextSelections]: s_Contexts) {
            auto it = std::ranges::find(contextSelections.begin(), contextSelections.end(), selectionID);
            if (it == contextSelections.end())
                continue;

            //Application::GetLink().DispatchEvent<SelectionChangedEvent>(contextID, selectionID, false);
            contextSelections.erase(it);
            break;
        }
    }

    void SelectionManager::Deselect(SelectionContext contextID, UUID selectionID) {
        auto& contextSelections = s_Contexts[contextID];
        auto it = std::ranges::find(contextSelections.begin(), contextSelections.end(), selectionID);
        if (it == contextSelections.end())
            return;

        contextSelections.erase(it);
    }

    void SelectionManager::DeselectAll() {
        for (auto& [ctxID, contextSelections]: s_Contexts) {
            //for (const auto& selectionID : contextSelections)
            //Application::GetLink().DispatchEvent<SelectionChangedEvent>(ctxID, selectionID, false);
            contextSelections.clear();
        }
    }

    void SelectionManager::DeselectAll(SelectionContext contextID) {
        auto& contextSelections = s_Contexts[contextID];

        //for (const auto& selectionID : contextSelections)
        //Application::GetLink().DispatchEvent<SelectionChangedEvent>(contextID, selectionID, false);

        contextSelections.clear();
    }

    UUID SelectionManager::GetSelection(SelectionContext context, size_t index) {
        auto& contextSelections = s_Contexts[context];
        AstraException::assertV(index < contextSelections.size(), "SelectionManager: invalid get selection");
        return contextSelections[index];
    }

    size_t SelectionManager::GetSelectionCount(SelectionContext contextID) {
        return s_Contexts[contextID].size();
    }

}
