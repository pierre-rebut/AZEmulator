//
// Created by pierr on 26/03/2023.
//
#include "AstraProject.h"

#include <fstream>

#include "ViewerConstants.h"
#include "Commons/utils/YAMLimport.h"
#include "Commons/utils/Utils.h"

#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"

#include "ViewerApp/Custom/CustomEvents/ProjectEvents.h"

#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/manager/running/RunManager.h"
#include "CpuEngine/manager/cpulib/CoreLibManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "CpuEngine/manager/buses/DataBusManager.h"

#include "ViewerApp/Custom/Serialization/DeviceLoader.h"
#include "ViewerApp/Custom/Serialization/EntitiesLoader.h"
#include "ViewerApp/Custom/Serialization/DataBusLoader.h"
#include "ViewerApp/Custom/Serialization/MemoryLoader.h"
#include "ViewerApp/Custom/Serialization/CpuLoader.h"
#include "ViewerApp/Custom/Serialization/ProjectLoader.h"
#include "ViewerApp/Custom/Serialization/LogLoader.h"
#include "ViewerApp/Custom/Serialization/NodesLoader.h"
#include "ViewerApp/CoreLib/Events/DelayedActionEvent.h"
#include "ViewerApp/CoreLib/AsyncJob.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "ViewerApp/Custom/CustomEvents/EditorEvents.h"
#include "ViewerApp/Custom/Serialization/AppLoader.h"

namespace Astra::UI::App {

    bool AstraProject::LoadProject(const std::filesystem::path& pProjectPath, bool pCreateOnFailed) noexcept {
        if (const auto project = Project::CurrentProject(); project && project->rootDirectory == pProjectPath) {
            LOG_WARN("[Project] Project already open", project->getProjectName());
            Core::Events::Get().OnEvent<Core::NotificationEvent>(
                    AstraMessage::New2(AstraMessageType::Warning, I18N::Get("PROJECT_ALREADY_OPEN"), project->getProjectName()));
            return false;
        }

        const std::filesystem::path& astraPath = pProjectPath / ViewerConstants::ASTRA_DIR;
        const std::filesystem::path& projectFilePath = astraPath / ViewerConstants::PROJECT_FILE;
        if (!std::filesystem::exists(projectFilePath)) {
            if (pCreateOnFailed) {
                return CreateProject(pProjectPath, "unknown", nullptr);
            }

            LOG_ERROR("[Project] Invalid project dir {}", pProjectPath.string());
            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Error, I18N::Get("INVALID_PROJECT_DIR")));
            return false;
        }

        if (!Core::FileManager::CreateDirectoryIfNotExists(astraPath / ViewerConstants::REG_DUMP_DIR) ||
            !Core::FileManager::CreateDirectoryIfNotExists(astraPath / ViewerConstants::MEM_DIR)) {

            LOG_ERROR("[Project] Can not create project dir {}", pProjectPath.string());
            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Error, I18N::Get("INVALID_PROJECT_RIGHTS")));
            return false;
        }

        Core::Events::Get().OnEvent<Core::DelayedActionEvent>("LoadProject", [pProjectPath](){
            loadProjectInternal(pProjectPath, false);
        });

        return true;
    }

    bool AstraProject::CreateProject(const std::filesystem::path& pProjectPath, const std::string& pProjectName, const char* templateId) noexcept {
        LOG_DEBUG("[Project] CreateProject");

        if (pProjectName.empty()) {
            LOG_ERROR("[Project] Invalid new project config");
            return false;
        }

        const std::filesystem::path& astraPath = pProjectPath / ViewerConstants::ASTRA_DIR;
        if (!std::filesystem::exists(pProjectPath)) {
            LOG_DEBUG("[Project] Creating new project directory");
            if (!Core::FileManager::CreateDirectoryA(pProjectPath)) {
                LOG_ERROR("[Project] Can not created project directory {}", pProjectPath.string());
                Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Error, I18N::Get("CAN_NOT_CREATE_DIR")));
                return false;
            }
        } else {
            if (std::filesystem::exists(astraPath)) {
                LOG_ERROR("[Project] directory already in used {}", astraPath);
                Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Error, I18N::Get("PROJECT_DIR_IN_USE")));
                return false;
            }
        }

        try {
            if (!Core::FileManager::CreateDirectoryA(astraPath)) {
                throw AstraException(I18N::Get("CAN_NOT_CREATE_ASTRA_DIR"));
            }

            createEmptyProject(pProjectName, astraPath);

            if (templateId) {
                createProjectFromTemplate(pProjectPath, templateId);
            }

            if (!Core::FileManager::CreateDirectoryIfNotExists(astraPath / ViewerConstants::MEM_DIR) ||
                !Core::FileManager::CreateDirectoryIfNotExists(astraPath / ViewerConstants::REG_DUMP_DIR)) {
                throw AstraException(I18N::Get("CAN_NOT_CREATE_SUB_ASTRA_DIR"));
            }

        } catch (const std::exception& e) {
            LOG_ERROR("[Project] Create error: {}", e.what());
            Project::SetCurrentProject(nullptr);

            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New(AstraMessageType::Error, I18N::Get("CREATE_PROJECT_FAILED"), e.what()));
            return false;
        }

        Core::Events::Get().OnEvent<Core::DelayedActionEvent>("CreateProject", [pProjectPath](){
            loadProjectInternal(pProjectPath, true);
        });

        LOG_INFO("[Project] CreateProject END");
        return true;
    }

    void AstraProject::createEmptyProject(const std::string& pProjectName, const std::filesystem::path& astraPath) {
        std::ofstream fileOut(astraPath / ViewerConstants::PROJECT_FILE);
        AstraException::assertV(fileOut.good(), I18N::Get("CAN_NOT_OPEN_PROJECT_FILE"),
                                astraPath / ViewerConstants::PROJECT_FILE);

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Version" << YAML::Value << 1;
        out << YAML::Key << "ProjectName" << YAML::Value << pProjectName;
        out << YAML::Key << "CurrentEngine" << YAML::Value << 0;

        out << YAML::Key << "Settings" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "ContentBrowserThumbnailSize" << YAML::Value << 128;
        out << YAML::Key << "ContentBrowserShowAssetTypes" << YAML::Value << true;
        out << YAML::Key << "LogLevel" << YAML::Value << "info";
        out << YAML::Key << "LogAutoScroll" << YAML::Value << false;
        out << YAML::Key << "LogAutoCleanOnRun" << YAML::Value << false;
        out << YAML::Key << "CppCompiler" << YAML::Value << "g++";
        out << YAML::Key << "ScreenMemoryAddress" << YAML::Value << 0x0;
        out << YAML::Key << "AutoSaving" << YAML::Value << true;
        out << YAML::EndMap;

        out << YAML::Key << "EditorFiles" << YAML::Value << YAML::BeginSeq << YAML::EndSeq;

        out << YAML::EndMap;
        fileOut << out.c_str();
        fileOut.close();
    }

    void AstraProject::createProjectFromTemplate(const std::filesystem::path& projectPath, const char* templateId) {
        LOG_DEBUG("[AstraProject] Creating from template {}", templateId);

        const std::filesystem::path& templatePath = std::filesystem::path("templates") / templateId;
        if (!std::filesystem::exists(templatePath)) {
            throw AstraException(I18N::Get("INVALID_TEMPLATE"), templateId);
        }
        
        Core::FileManager::CopyDirectory(templatePath, projectPath);
    }

    void AstraProject::loadProjectInternal(const std::filesystem::path& pProjectPath, bool openReadme) noexcept {
        try {
            Project::SetCurrentProject(nullptr);
            Project::SetCurrentProject(CreateRef<AstraProject>("unknown", pProjectPath));

            Core::AsyncJob::Get().PushTask(&AstraProject::initProject, AstraProject::CurrentProject(), openReadme);
        } catch (const std::exception& e) {
            LOG_ERROR("[Project] Load error: {}", e.what());
            Project::SetCurrentProject(nullptr);
            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New(AstraMessageType::Error, I18N::Get("LOAD_PROJECT_FAILED"), e.what()));
        }
    }

    AstraProject::AstraProject(std::string projectName, std::filesystem::path mainDirectory) : Project(std::move(projectName), std::move(mainDirectory)) {
        LOG_DEBUG("[Project] Init project {}", getProjectName());

        const auto projectConfigFile = rootDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::PROJECT_FILE;
        auto res = ProjectLoader::loadConfig(projectConfigFile, this);
        AstraException::assertV(res, I18N::Get("INVALID_PROJECT_CONFIG"), projectConfigFile.string());

        const auto appConfigFile = rootDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::APP_FILE;
        res = AppLoader::loadConfig(appConfigFile, this);
        AstraException::assertV(res, I18N::Get("INVALID_PROJECT_CONFIG"), appConfigFile.string());

        Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Success, I18N::Get("PROJECT_LOADED"), getProjectName()));
        LOG_DEBUG("[Project] Init project END");
    }

    AstraProject::~AstraProject() {
        LOG_DEBUG("[Project] close project {}", getProjectName());

        CPU::Core::RunManager::Get().ClearAll();
        CPU::Core::DataBusManager::Get().RemoveAllBus();
        CPU::Core::DevicesManager::Get().Reset();
        CPU::Core::EngineManager::Get().ClearEngines();
        CPU::Core::CoreLibManager::Get().Reset();

        LOG_DEBUG("[Project] close project END");
    }

    inline static void checkError(const std::string& checkName, bool isSuccess) {
        AstraException::assertV(isSuccess, checkName);
    }

    void AstraProject::initProject(bool openReadme) noexcept {
        LOG_DEBUG("[PROJECT] initProject");

        try {
            const std::filesystem::path& astraDir = rootDirectory / ViewerConstants::ASTRA_DIR;

            UUID lastCurrentEngine;
            std::vector<CPU::Core::CpuCreateData> cpuData;
            std::vector<CPU::Core::BusCreateData> busData;
            std::vector<CPU::Core::DeviceCreateData> devicesData;

            checkError("CpuLoad", CpuLoader::loadConfig(astraDir / ViewerConstants::CPU_CONFIG, cpuData, lastCurrentEngine));
            checkError("Device", DeviceLoader::loadConfig(astraDir / ViewerConstants::DEVICES_CONFIG, devicesData));
            checkError("Bus", DataBusLoader::loadConfig(astraDir / ViewerConstants::DATABUS_CONFIG, busData));
            checkError("Log", LogLoader::loadConfig(astraDir / ViewerConstants::LOG_FILE, this));
            checkError("Nodes", NodesLoader::loadConfig(astraDir / ViewerConstants::NODE_FILE, m_data));

            CPU::Core::EngineManager::Get().InitEngines(cpuData);
            CPU::Core::DevicesManager::Get().Init(devicesData);
            CPU::Core::DataBusManager::Get().Init(busData);

            MemoryLoader::loadAllMemories(rootDirectory);

            for (const auto& [uuid, engine]: CPU::Core::EngineManager::Get().getEngines()) {
                checkError("Entities", EntitiesLoader::loadConfig(engine, astraDir / ViewerConstants::REG_DUMP_DIR / myFormat("{}.yml", uuid)));
            }

            m_currentCpuEngine = CPU::Core::EngineManager::Get().GetEngineByUUID(lastCurrentEngine);
        } catch (const std::exception& e) {
            LOG_ERROR("[Project] Init project error: {}", e.what());
            m_currentCpuEngine = nullptr;
            Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New(AstraMessageType::Error, I18N::Get("INIT_PROJECT_FAILED"), e.what()));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        m_isProjectLoaded = true;

        Core::Events::Get().OnEvent<ProjectLoadedEvent>();
        Core::Events::Get().OnEvent<CpuChangedEvent>();

        if (openReadme) {
            const auto& readme = Core::AssetManager::Get().GetMetadata("README.md");
            if (readme.isValid()) {
                Core::Events::Get().OnEvent<EditorOpenEvent>(readme, true);
                m_forceFocusEditor = true;
            }
        }

        LOG_DEBUG("[PROJECT] initProject END");
    }

    void AstraProject::SaveProject() noexcept {
        LOG_DEBUG("[PROJECT] SaveProject");

        Core::AsyncJob::Get().PushTask(&AstraProject::saveProjectInternal, this);

        LOG_DEBUG("[PROJECT] SaveProject");
    }

    void AstraProject::saveProjectInternal() noexcept {
        LOG_DEBUG("[PROJECT] SaveProjectInternal");

        const auto astraDir = rootDirectory / ViewerConstants::ASTRA_DIR;

        ProjectLoader::saveConfig(astraDir / ViewerConstants::PROJECT_FILE, this);
        AppLoader::saveConfig(astraDir / ViewerConstants::APP_FILE, this);
        CpuLoader::saveConfig(astraDir / ViewerConstants::CPU_CONFIG, this);

        for (const auto& [uuid, engine]: CPU::Core::EngineManager::Get().getEngines()) {
            EntitiesLoader::saveValues(engine, astraDir / ViewerConstants::REG_DUMP_DIR / myFormat("{}.yml", uuid));
        }

        LogLoader::saveConfig(astraDir / ViewerConstants::LOG_FILE, this);

        MemoryLoader::saveAllMemories(rootDirectory);
        DataBusLoader::saveConfig(astraDir / ViewerConstants::DATABUS_CONFIG);
        DeviceLoader::saveConfig(astraDir / ViewerConstants::DEVICES_CONFIG);
        NodesLoader::saveConfig(astraDir / ViewerConstants::NODE_FILE, m_data);

        Core::Events::Get().OnEvent<Core::NotificationEvent>(AstraMessage::New2(AstraMessageType::Success, I18N::Get("PROJECT_SAVED"), getProjectName()));
        LOG_DEBUG("[PROJECT] SaveProjectInternal END");
    }
}
