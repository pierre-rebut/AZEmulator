//
// Created by pierr on 10/08/2023.
//
#pragma once

#include <vector>
#include <algorithm>
#include "Commons/utils/UUID.h"
#include "Commons/AstraException.h"

namespace Astra::UI::App {

    class SelectionStack
    {
    public:
        void CopyFrom(const SelectionStack& other) {
            m_Selections.assign(other.begin(), other.end());
        }

        void CopyFrom(const std::vector<UUID>& other) {
            m_Selections.assign(other.begin(), other.end());
        }

        void Select(UUID handle) {
            if (IsSelected(handle)) {
                return;
            }

            m_Selections.push_back(handle);
        }

        void Deselect(UUID handle) {
            if (!IsSelected(handle)) {
                return;
            }

            for (auto it = m_Selections.begin(); it != m_Selections.end(); it++) {
                if (handle == *it) {
                    m_Selections.erase(it);
                    break;
                }
            }
        }

        bool IsSelected(UUID handle) const {
            return std::ranges::any_of(m_Selections,
                                       [handle](UUID selectedHandle) { return selectedHandle == handle; });
        }

        void Clear() {
            m_Selections.clear();
        }

        size_t SelectionCount() const { return m_Selections.size(); }

        const UUID* SelectionData() const { return m_Selections.data(); }

        UUID operator[](size_t index) const {
            AstraException::assertV(index < m_Selections.size(), "SelectionStack: invalid index");
            return m_Selections[index];
        }

        std::vector<UUID>::iterator begin() { return m_Selections.begin(); }

        std::vector<UUID>::const_iterator begin() const { return m_Selections.begin(); }

        std::vector<UUID>::iterator end() { return m_Selections.end(); }

        std::vector<UUID>::const_iterator end() const { return m_Selections.end(); }

    private:
        std::vector<UUID> m_Selections{};
    };

}
