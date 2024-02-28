//
// Created by pierr on 30/07/2023.
//

#include "CoreLib.h"

#include "Commons/Log.h"

#include "CpuEngine/exception/CppException.h"

namespace Astra::CPU::Core {

    CoreLib::CoreLib(std::string pDir, std::string pName, std::filesystem::path pPath)
            : dir(std::move(pDir)), name(std::move(pName)), path(std::move(pPath)) {
        LOG_CPU_INFO("[CoreLib] Init core lib {}/{} ({})", dir, name, path);
    }

    CoreLib::~CoreLib() {
        Reset();
    }

    void CoreLib::Reset() {
        if (!m_lib) {
            return;
        }

        unloadEnginesCore();
        m_engines.clear();
        resetSharedLib();
    }

    bool CoreLib::ReloadLibrary() noexcept {
        LOG_CPU_DEBUG("[CoreLib] ReloadLibrary {}/{}", dir, name);

        if (m_engines.empty()) {
            return false;
        }

        try {
            unloadEnginesCore();

            initSharedLib();

            loadEnginesCore();
        } catch (const std::exception& e) {
            auto astraMsg = AstraMessage::New(AstraMessageType::Error);
            astraMsg->setTitle("Load lib {}/{} failed", dir, name);
            astraMsg->setContent(e.what());
            LogManager::Get().Notify(astraMsg);
            LOG_CPU_DEBUG("[CoreLib] ReloadLibrary END");
            return false;
        }

        LOG_CPU_DEBUG("[CoreLib] ReloadLibrary END");
        return true;
    }

    void CoreLib::AddEngine(const Ref<CpuEngine>& engine) {
        LOG_CPU_DEBUG("[CoreLib] AddEngine {} to lib {}/{}", engine->deviceUUID, dir, name);

        if (!IsLoaded()) {
            initSharedLib();
        }

        m_engines[engine->deviceUUID] = engine;
        engine->LoadCore(m_coreFactory);

        LOG_CPU_DEBUG("[CoreLib] AddEngine END");
    }

    void CoreLib::RemoveEngine(const Ref<CpuEngine>& engine) {
        LOG_CPU_DEBUG("[CoreLib] RemoveEngine {}", engine->deviceUUID);

        engine->UnloadCore();
        m_engines.erase(engine->deviceUUID);

        if (m_engines.empty()) {
            LOG_CPU_DEBUG("[CoreLib] Unloading unused lib {}/{}", dir, name);
            resetSharedLib();
        }

        LOG_CPU_DEBUG("[CoreLib] RemoveEngine END");
    }

    void CoreLib::initSharedLib() {
        LOG_CPU_DEBUG("[CoreLib] initSharedLib {}/{}", dir, name);

        m_lib = CreateScope<SharedLibrary>(path);

        using CoreFactoryFn = IFactory*();
        m_coreFactory = m_lib->get<CoreFactoryFn>(CREATE_CORE_FN)();

        LOG_CPU_DEBUG("[CoreLib] initSharedLib END");
    }

    void CoreLib::resetSharedLib() {
        LOG_CPU_DEBUG("[CoreLib] resetSharedLib {}/{}", dir, name);

        m_coreFactory = nullptr;
        m_lib = nullptr;

        LOG_CPU_DEBUG("[CoreLib] resetSharedLib END");
    }

    void CoreLib::loadEnginesCore() const {
        LOG_CPU_DEBUG("[CoreLib] loadEnginesCore {}/{}", dir, name);

        for (const auto& [engineUUID, engine]: m_engines) {
            engine->LoadCore(m_coreFactory);
        }

        LOG_CPU_DEBUG("[CoreLib] loadEnginesCore END");
    }

    void CoreLib::unloadEnginesCore() const {
        LOG_CPU_DEBUG("[CoreLib] unloadEnginesCore {}/{}", dir, name);

        for (const auto& [engineUUID, engine]: m_engines) {
            engine->UnloadCore();
        }

        LOG_CPU_DEBUG("[CoreLib] unloadEnginesCore END");
    }
}
