//
// Created by pierr on 30/07/2023.
//
#pragma once

#include <map>
#include <filesystem>

#include "EngineLib/data/Base.h"

#include "CpuEngine/manager/cpulib/lib/CoreLib.h"
#include "CpuEngine/engine/cpu/CpuEngine.h"

namespace Astra::CPU::Core {

    struct CoreLibDir
    {
        const std::string name;
        std::map<std::string, Ref<CoreLib>> libs{};
    };

    class CoreLibManager : public Singleton<CoreLibManager>
    {
    public:
        static constexpr const char* NAME = "CoreLibManager";

    private:
        std::map<std::string, Scope<CoreLibDir>> m_libList;

    public:
        void FindCoreLib(const std::filesystem::path& libDirectory);
        void Reset() const;

        void SetAndLoadEngineCoreLib(const std::string& dirName, const std::string& libName, const Ref<CpuEngine>& engine) const;
        void UnsetAndUnloadEngineCoreLib(const Ref<CpuEngine>& engine) const;

        void ReloadAllLibrary() const;

        bool ReloadLibraryByName(const std::string& dirName, const std::string& name);

        inline const auto& GetCoreLibs() const {return m_libList;}

        bool LoadEngineCoreLib(const Ref<CpuEngine>& engine) const noexcept;
        void UnloadEngineCoreLib(const Ref <CpuEngine>& engine) const noexcept;
    };

}
