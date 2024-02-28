//
// Created by pierr on 24/03/2023.
//
#pragma once

#include "Commons/utils/UUID.h"
#include "CpuEngine/manager/LogManager.h"

namespace Astra::CPU::Core {

    class CpuLog
    {
    public:
        const UUID cpuUUID;
        const std::string& cpuName;

    private:
        LogManager& m_logManager = LogManager::Get();

    public:
        CpuLog(UUID uuid, const std::string& name) : cpuUUID(uuid), cpuName{name} {}

        template<typename... Args>
        inline void error(const std::string_view& fmt, Args&& ... args) const {
            Notify(AstraMessageType::Error, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void warn(const std::string_view& fmt, Args&& ... args) const {
            Notify(AstraMessageType::Warning, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void info(const std::string_view& fmt, Args&& ... args) const {
            Notify(AstraMessageType::Info, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void success(const std::string_view& fmt, Args&& ... args) const {
            Notify(AstraMessageType::Success, fmt, std::forward<Args>(args)...);
        }

    private:
        template<typename... Args>
        inline void Notify(AstraMessageType type, const std::string_view& content, Args&& ... args) const {
            m_logManager.Notify(AstraMessage::New(type, myFormat("CPU {} ({})", cpuName, cpuUUID), content, std::forward<Args>(args)...));
        }
    };

}
