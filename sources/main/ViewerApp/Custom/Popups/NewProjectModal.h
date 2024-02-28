//
// Created by pierr on 20/06/2023.
//
#pragma once

#include <filesystem>
#include "ViewerApp/CoreLib/Windows/APopup.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    struct ProjectTemplate {
        const char* id;
        const char* icon;
        const char* name;
        const char* description;
    };

    class NewProjectModal : public UI::Core::AModal
    {
    public:
        static const ProjectTemplate TEMPLATE_LIST[];

    private:
        std::string m_projectName = "untitled";
        std::string m_projectTmpPath = "C:\\";
        std::filesystem::path m_projectPath;
        const ProjectTemplate* m_template = &(TEMPLATE_LIST[0]);

    public:
        static constexpr const char* NAME = ICON_FA_FILE " New project";

        NewProjectModal() : AModal(NAME) {}

    private:
        void contentOutliner();
        void contentNewProject();

        void drawPopupContent() override;
    };

}
