//
// Created by pierr on 30/07/2023.
//
#include "CoreLibManager.h"

#include "Commons/Log.h"
#include "CpuEngine/exception/EngineException.h"
#include "CpuEngine/manager/EngineManager.h"

namespace Astra::CPU::Core {

    void CoreLibManager::FindCoreLib(const std::filesystem::path& libDirectory) {
        LOG_CPU_DEBUG("[CoreLibManager] FindCoreLib");

        m_libList.clear();

        for (const auto& dirEntry: std::filesystem::directory_iterator(libDirectory)) {
            const auto& dirPath = dirEntry.path();

            if (!dirEntry.is_directory()) {
                LOG_CPU_DEBUG("[CoreLibManager] {} not a directory, skipped.", dirPath.filename());
                continue;
            }

            const auto dirName = dirPath.filename().string();
            auto newDirEntry = CreateScope<CoreLibDir>(dirName);

            for (const auto& libEntry: std::filesystem::directory_iterator(dirPath)) {
                const auto& libPath = libEntry.path();

                if (libEntry.is_directory() || (libPath.extension() != ".dll" && libPath.extension() != ".so")) {
                    LOG_CPU_WARN("[CoreLibManager] Invalid lib entry {}, skipped", libPath);
                    continue;
                }

                const auto libName = libPath.stem().string().substr(3);
                newDirEntry->libs[libName] = CreateRef<CoreLib>(dirName, libName, libPath);
            }

            if (!newDirEntry->libs.empty()) {
                m_libList[dirName] = std::move(newDirEntry);
            }
        }

        LOG_CPU_DEBUG("[CoreLibManager] FindCoreLib END");
    }

    void CoreLibManager::Reset() const {
        LOG_CPU_DEBUG("[CoreLibManager] Reset");

        for (const auto& [dirName, dirEntry]: m_libList) {
            for (const auto& [libName, lib]: dirEntry->libs) {
                lib->Reset();
            }
        }

        LOG_CPU_DEBUG("[CoreLibManager] Reset END");
    }

    void CoreLibManager::ReloadAllLibrary() const {
        LOG_CPU_DEBUG("[CoreLibManager] ReloadAllLibrary");

        for (const auto& [dirName, dirEntry]: m_libList) {
            for (const auto& [libName, lib]: dirEntry->libs) {
                lib->ReloadLibrary();
            }
        }

        LOG_CPU_DEBUG("[CoreLibManager] ReloadAllLibrary END");
    }

    void CoreLibManager::SetAndLoadEngineCoreLib(const std::string& dirName, const std::string& libName, const Ref<CpuEngine>& engine) const {
        LOG_CPU_DEBUG("[CoreLibManager] Set core lib {}/{} to engine {}", dirName, libName, engine->deviceUUID);

        UnloadEngineCoreLib(engine);
        engine->SetCpuCoreName(dirName, libName);
        LoadEngineCoreLib(engine);

        LOG_CPU_DEBUG("[CoreLibManager] SetAndLoadEngineCoreLib END");
    }

    bool CoreLibManager::LoadEngineCoreLib(const Ref<CpuEngine>& engine) const noexcept {
        const auto& [dirName, libName] = engine->GetCpuCoreName();
        if (dirName.empty() || libName.empty()) {
            return false;
        }

        LOG_CPU_DEBUG("[CoreLibManager] Load core lib {}/{} to engine {}", dirName, libName, engine->deviceUUID);

        try {
            EngineException::assertV(m_libList.contains(dirName), "Can not find dir {}", dirName);
            const auto& libs = m_libList.at(dirName)->libs;
            EngineException::assertV(libs.contains(libName), "Can not find lib {} in dir {}", libName, dirName);

            libs.at(libName)->AddEngine(engine);
        } catch (const std::exception& e) {
            LOG_CPU_ERROR("[CoreLibManager] LoadEngineCoreLib error: {}", e.what());
            NOTIFY_ERROR("Core Lib manager", e.what());
            return false;
        }

        LOG_CPU_DEBUG("[CoreLibManager] LoadEngineCoreLib END");
        return true;
    }

    void CoreLibManager::UnsetAndUnloadEngineCoreLib(const Ref<CpuEngine>& engine) const {
        LOG_CPU_DEBUG("[CoreLibManager] UnsetAndUnloadEngineCoreLib {}", engine->deviceUUID);

        UnloadEngineCoreLib(engine);
        engine->SetCpuCoreName("", "");

        LOG_CPU_DEBUG("[CoreLibManager] UnsetAndUnloadEngineCoreLib END");
    }

    void CoreLibManager::UnloadEngineCoreLib(const Ref<CpuEngine>& engine) const noexcept {
        const auto& [dirName, libName] = engine->GetCpuCoreName();
        if (dirName.empty() || libName.empty()) {
            return;
        }

        LOG_CPU_DEBUG("[CoreLibManager] UnloadEngineCoreLib {}", engine->deviceUUID);

        try {
            EngineException::assertV(m_libList.contains(dirName), "Can not find dir {}", dirName);
            const auto& libs = m_libList.at(dirName)->libs;
            EngineException::assertV(libs.contains(libName), "Can not find lib {} in dir {}", libName, dirName);

            libs.at(libName)->RemoveEngine(engine);
        } catch (const std::exception& e) {
            LOG_CPU_ERROR("[CoreLibManager] UnloadEngineCoreLib error: {}", e.what());
            NOTIFY_ERROR("Core Lib manager", e.what());
        }

        LOG_CPU_DEBUG("[CoreLibManager] UnloadEngineCoreLib END");
    }

    bool CoreLibManager::ReloadLibraryByName(const std::string& dirName, const std::string& libName) {
        LOG_CPU_DEBUG("[CoreLibManager] Reload lib {} in dir {}", libName, dirName);

        EngineException::assertV(m_libList.contains(dirName), "Can not find dir {}", dirName);
        const auto& libs = m_libList.at(dirName)->libs;
        EngineException::assertV(libs.contains(libName), "Can not find lib {} in dir {}", libName, dirName);

        libs.at(libName)->ReloadLibrary();

        NOTIFY_SUCCESS("Core Lib manager", "Core lib {} reloaded", libName);

        LOG_CPU_DEBUG("[CoreLibManager] ReloadLibraryByName END");
        return true;
    }
}
