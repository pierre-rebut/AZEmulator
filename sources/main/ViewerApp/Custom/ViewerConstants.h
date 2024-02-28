//
// Created by pierr on 25/03/2023.
//
#pragma once
namespace Astra::UI::App {

    class ViewerConstants
    {
    public:
        ViewerConstants() = delete;

        static constexpr const char* CONFIG_RECENTLY_PROJECT_FILE = "projects-history.yml";

        static constexpr const char* ASTRA_DIR = ".astra";

        static constexpr const char* REGISTRY_FILE = "registry.yml";
        static constexpr const char* PROJECT_FILE = "project.yml";
        static constexpr const char* APP_FILE = "app.yml";
        static constexpr const char* LOG_FILE = "log-history.yml";
        static constexpr const char* NODE_FILE = "nodes.yml";
        static constexpr const char* TIPS_FILE = "tips-history.yml";
        static constexpr const char* LANG_FILE = "lang.yml";

        static constexpr const char* CPU_CONFIG = "cpu-config.yml";
        static constexpr const char* DEVICES_CONFIG = "devices-config.yml";
        static constexpr const char* DATABUS_CONFIG = "bus-config.yml";

        static constexpr const char* MEM_DIR = "memories";
        static constexpr const char* REG_DUMP_DIR = "registers";

        static constexpr const char* CORE_LIB_DIR = "./CoreLibrary";
        static constexpr const char* I18N_DIR = "./i18n";
    };

}
