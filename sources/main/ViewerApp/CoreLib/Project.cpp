//
// Created by pierr on 21/08/2023.
//

#include "Project.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"

#include "Commons/Log.h"
#include "ViewerApp/CoreLib/Windows/WindowsManager.h"

namespace Astra::UI::Core {

    Project::Project(std::string pProjectName, std::filesystem::path pRootDirectory)
    : m_projectName(std::move(pProjectName)), rootDirectory(std::move(pRootDirectory)) {
        LOG_DEBUG("[Project] Init project");

        UI::Core::AssetManager::Get().Init(this);
    }

    Project::~Project() {
        LOG_DEBUG("[Project] destroy project");

        UI::Core::AssetManager::Get().Shutdown(this);

        for (const auto& popup : UI::Core::WindowsManager::Get().getPopups()) {
            popup.second->Reset();
        }
    }
}