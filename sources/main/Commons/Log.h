//
// Created by pierr on 11/09/2021.
//

#pragma once

#include "EngineLib/data/Base.h"

#pragma warning(push, 0)

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#undef LoadLibrary

#pragma warning(pop)

namespace Astra {
    enum class LogLevel
    {
        trace = SPDLOG_LEVEL_TRACE,
        debug = SPDLOG_LEVEL_DEBUG,
        info = SPDLOG_LEVEL_INFO,
        warn = SPDLOG_LEVEL_WARN,
        err = SPDLOG_LEVEL_ERROR,
        critical = SPDLOG_LEVEL_CRITICAL,
        off = SPDLOG_LEVEL_OFF
    };

    class Log
    {
    public:
        static void Init();

        static void SetLog(LogLevel pLevel);

        inline static Ref<spdlog::logger>& GetLogger() { return sLogger; }

        inline static Ref<spdlog::logger>& GetCpuLogger() { return sCpuLogger; }

    private:
        static Ref<spdlog::logger> sLogger;
        static Ref<spdlog::logger> sCpuLogger;

        static void initFirstConfigFile();

        static void loadLogConfigFile();
    };
}

#ifdef LOG_ENABLE
#define LOG_INIT() Astra::Log::Init()
#define LOG_DEBUG(...) ::Astra::Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_TRACE(...) ::Astra::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::Astra::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::Astra::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Astra::Log::GetLogger()->error(__VA_ARGS__)

#define LOG_CPU_DEBUG(...) ::Astra::Log::GetCpuLogger()->debug(__VA_ARGS__)
#define LOG_CPU_TRACE(...) ::Astra::Log::GetCpuLogger()->trace(__VA_ARGS__)
#define LOG_CPU_INFO(...) ::Astra::Log::GetCpuLogger()->info(__VA_ARGS__)
#define LOG_CPU_WARN(...) ::Astra::Log::GetCpuLogger()->warn(__VA_ARGS__)
#define LOG_CPU_ERROR(...) ::Astra::Log::GetCpuLogger()->error(__VA_ARGS__)
#else
#define LOG_INIT()
#define LOG_DEBUG(...)
#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_CPU_DEBUG(...)
#define LOG_CPU_TRACE(...)
#define LOG_CPU_INFO(...)
#define LOG_CPU_WARN(...)
#define LOG_CPU_ERROR(...)
#endif