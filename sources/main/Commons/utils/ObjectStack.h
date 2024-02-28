//
// Created by pierr on 05/03/2022.
//

#pragma once

#include "EngineLib/data/Base.h"
#include "Commons/AObject.h"

#include <map>

namespace Astra {

    template<class OBJ = AObject>
    class ObjectStack
    {
        using PanelStackType = std::map<std::string, Scope<OBJ>>;
        PanelStackType m_layers;

    public:
        explicit ObjectStack() = default;

        void clear() {
            m_layers.clear();
        }

        template<class T = OBJ, typename... Args>
        T* push(Args&& ... pArgs) {
            auto layer = CreateScope<T>(std::forward<Args>(pArgs)...);
            auto ptr = layer.get();
            m_layers.emplace(std::move(T::NAME), std::move(layer));
            return ptr;
        }

        template<class T = OBJ>
        T* get() {
            return dynamic_cast<T*>(m_layers.at(T::NAME).get());
        }

        PanelStackType::iterator begin() { return m_layers.begin(); }

        PanelStackType::iterator end() { return m_layers.end(); }

        PanelStackType::const_iterator begin() const { return m_layers.begin(); }

        PanelStackType::const_iterator end() const { return m_layers.end(); }
    };

}
