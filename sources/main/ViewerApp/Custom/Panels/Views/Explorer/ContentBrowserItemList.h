//
// Created by pierr on 10/08/2023.
//
#pragma once

#include <mutex>
#include <vector>

#include "EngineLib/data/Base.h"
#include "ViewerApp/Custom/Panels/Views/Explorer/item/ContentBrowserItem.h"

namespace Astra::UI::App {

    struct ContentBrowserItemList
    {
        static constexpr size_t InvalidItem = std::numeric_limits<size_t>::max();

        std::vector<Ref<ContentBrowserItem>> Items;

        std::vector<Ref<ContentBrowserItem>>::iterator begin() { return Items.begin(); }

        std::vector<Ref<ContentBrowserItem>>::iterator end() { return Items.end(); }

        std::vector<Ref<ContentBrowserItem>>::const_iterator begin() const { return Items.begin(); }

        std::vector<Ref<ContentBrowserItem>>::const_iterator end() const { return Items.end(); }

        Ref<ContentBrowserItem>& operator[](size_t index) { return Items[index]; }

        const Ref<ContentBrowserItem>& operator[](size_t index) const { return Items[index]; }

        ContentBrowserItemList() = default;

        ContentBrowserItemList(const ContentBrowserItemList& other) : Items(other.Items) {
        }

        ~ContentBrowserItemList() = default;

        ContentBrowserItemList& operator=(const ContentBrowserItemList& other) {
            Items = other.Items;
            return *this;
        }

        void Clear() {
            std::scoped_lock<std::mutex> lock(m_Mutex);
            Items.clear();
        }

        void erase(UUID handle) {
            size_t index = FindItem(handle);
            if (index == InvalidItem) {
                return;
            }

            std::scoped_lock<std::mutex> lock(m_Mutex);
            auto it = Items.begin() + index;
            Items.erase(it);
        }

        size_t FindItem(UUID handle) {
            if (Items.empty()) {
                return InvalidItem;
            }

            std::scoped_lock<std::mutex> lock(m_Mutex);
            for (size_t i = 0; i < Items.size(); i++) {
                if (Items[i]->GetID() == handle) {
                    return i;
                }
            }

            return InvalidItem;
        }

    private:
        std::mutex m_Mutex;
    };

}
