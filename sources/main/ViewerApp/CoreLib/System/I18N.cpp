//
// Created by pierr on 05/01/2024.
//

#include "I18N.h"
#include "ViewerApp/Custom/ViewerConstants.h"
#include "Commons/Log.h"

#include <filesystem>
#include "Commons/utils/YAMLimport.h"
#include "Commons/AstraException.h"
#include "Commons/utils/Utils.h"

namespace Astra::UI {

    static bool isFrench() {
#ifdef WIN32
        union LangId
        {
            struct
            {
                char primary;
                char secondary;
            };
            short id;
        };

        LangId lang;
        lang.id = GetUserDefaultUILanguage();
        return lang.primary == 12;
#else
        auto lang = getenv("LANG");
        if (!lang) {
            return false;
        }

        std::string langStr(lang);
        return langStr.starts_with("fr_FR");
#endif
    }

    const char* I18N::Get(const std::string& key) {
        return I18NImpl::Get().Trad(key);
    }

    I18NImpl::I18NImpl() {
        for (const auto& tradFile: std::filesystem::recursive_directory_iterator(UI::App::ViewerConstants::I18N_DIR)) {
            if (tradFile.is_directory() || tradFile.path().extension().string() != ".yml") {
                continue;
            }

            m_tradList.emplace_back(Utils::RemoveExtension(tradFile.path().filename().string()));
        }

        if (std::filesystem::exists(UI::App::ViewerConstants::LANG_FILE)) {
            std::ifstream file(UI::App::ViewerConstants::LANG_FILE);
            if (file.is_open()) {
                file >> m_currentTrad;
                file.close();

                if (std::find(m_tradList.begin(), m_tradList.end(), m_currentTrad) != m_tradList.end()) {
                    LoadTrad(m_currentTrad);
                    return;
                }
            }
        }

        if (isFrench()) {
            LoadTrad("fr_FR");
        } else {
            LoadTrad("en_EN");
        }
    }

    I18NImpl::~I18NImpl() {
        std::ofstream file(UI::App::ViewerConstants::LANG_FILE);
        if (file.is_open()) {
            file << m_currentTrad;
            file.close();
        }
    }

    bool I18NImpl::LoadTrad(const std::string& tradName) noexcept {
        const auto path = std::filesystem::path(UI::App::ViewerConstants::I18N_DIR) / (tradName + ".yml");
        if (!std::filesystem::exists(path)) {
            return false;
        }

        std::unordered_map<std::string, std::string> tmpTrad;

        try {
            YAML::Node data = YAML::LoadFile(path.string());

            for (const auto item: data) {
                auto key = item.first.as<std::string>();
                auto value = item.second.as<std::string>();

                tmpTrad[key] = value;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("I18N : load trad failed: {}", e.what());
            return false;
        }

        m_trad = std::move(tmpTrad);
        m_currentTrad = tradName;
        return true;
    }

    const char* I18NImpl::Trad(const std::string& key) {
        if (!m_trad.contains(key)) {
            m_trad[key] = "#" + key;
        }

        return m_trad.at(key).c_str();
    }

    void I18NImpl::ExportTrad() noexcept {
        const auto path = std::filesystem::path(UI::App::ViewerConstants::I18N_DIR) / (m_currentTrad + ".yml");
        try {
            YAML::Emitter out;
            out << YAML::BeginMap;

            for (const auto& [key, val]: m_trad) {
                out << YAML::Key << key << YAML::Value << val;
            }

            out << YAML::EndMap;
            std::ofstream fileOut(path);
            AstraException::assertV(fileOut.good(), "can not open file {}", path);
            fileOut << out.c_str();
            fileOut.close();

            LOG_INFO("ExportTrad : success");
        } catch (const std::exception& e) {
            LOG_WARN("ExportTrad error: {}", e.what());
        }
    }
}
