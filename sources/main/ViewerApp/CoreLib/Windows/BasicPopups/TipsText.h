//
// Created by pierr on 10/01/2024.
//
#pragma once

#include <unordered_map>
#include <string>

#include "Commons/utils/Singleton.h"

namespace Astra::UI {

    class TipsText : public Singleton<TipsText>
    {
    private:
        bool m_isTipsEnable = true;
        std::unordered_map<std::string, bool> m_isTipsVisible;

    public:
        static constexpr const char* NAME = "TipsText";

        TipsText();
        ~TipsText() override;

        void Show(const char* tipsId);

        bool IsEnable() const {return m_isTipsEnable;}
        void SetEnable(bool val) {
            m_isTipsEnable = val;
            if (val) {
                m_isTipsVisible.clear();
            }
        }
    };

}
