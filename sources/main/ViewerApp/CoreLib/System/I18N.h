//
// Created by pierr on 05/01/2024.
//

#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "Commons/utils/Singleton.h"

namespace Astra::UI {
    class I18NImpl : public Singleton<I18NImpl>
    {
    public:
        static constexpr const char* NAME = "I18N";

    private:
        std::vector<std::string> m_tradList;
        std::string m_currentTrad;
        std::unordered_map<std::string, std::string> m_trad;

    public:
        I18NImpl();
        ~I18NImpl() override;

        bool LoadTrad(const std::string& tradPath) noexcept;

        const std::string& Current() { return m_currentTrad; }

        const std::vector<std::string>& TradList() { return m_tradList; }

        const char* Trad(const std::string& key);

        void ExportTrad() noexcept;
    };

    class I18N
    {
    public:
        I18N() = delete;
        static const char* Get(const std::string& key);
    };
}