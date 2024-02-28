//
// Created by pierr on 30/07/2023.
//
#pragma once

#include "EngineLib/data/Base.h"
#include "Commons/utils/UUID.h"
#include "Commons/utils/SharedLibrary.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"
#include "EngineLib/IFactory.h"

namespace Astra::CPU::Core {

    class CoreLib
    {
    public:
        const std::string dir;
        const std::string name;
        const std::filesystem::path path;

        static constexpr const char* CREATE_CORE_FN = "CoreFactory";

    private:
        Scope<SharedLibrary> m_lib;
        const IFactory* m_coreFactory = nullptr;

        std::unordered_map<UUID, Ref<CpuEngine>> m_engines;

    public:
        CoreLib(std::string pDir, std::string pName, std::filesystem::path pPath);
        ~CoreLib();

        bool ReloadLibrary() noexcept;
        void Reset();

        void AddEngine(const Ref<CpuEngine>& engine);
        void RemoveEngine(const Ref<CpuEngine>& engine);

        inline bool IsLoaded() const {return m_coreFactory;}
        inline const std::unordered_map<UUID, Ref<CpuEngine>>& GetEngines() const {return m_engines;}

    private:
        void initSharedLib();
        void resetSharedLib();

        void loadEnginesCore() const;
        void unloadEnginesCore() const;
    };

}
