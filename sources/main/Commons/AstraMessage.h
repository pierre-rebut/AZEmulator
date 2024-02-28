//
// Created by pierr on 22/08/2023.
//

#pragma once

#include <string>
#include <chrono>

#include "EngineLib/data/Base.h"
#include "format.h"

namespace Astra {

    enum class AstraMessageType
    {
        Error = 0,
        Warning,
        Success,
        Info,
        Debug
    };

    class AstraMessage
    {
    public:
        const AstraMessageType type = AstraMessageType::Info;

    private:
        std::string m_title;
        std::string m_content;
        std::chrono::system_clock::time_point m_timing = std::chrono::system_clock::now();

    public:
        static inline Ref<AstraMessage> New(AstraMessageType mType = AstraMessageType::Info) {
            return Ref<AstraMessage>(new AstraMessage(mType));
        }

        template<typename... Args>
        static inline Ref<AstraMessage> New2(AstraMessageType mType, const std::string_view& fmt, Args&& ... args) {
            return Ref<AstraMessage>(new AstraMessage(mType, myFormat(fmt, std::forward<Args>(args)...)));
        }

        template<typename... Args>
        static inline Ref<AstraMessage> New(AstraMessageType mType, std::string&& title, const std::string_view& fmt, Args&& ... args) {
            return Ref<AstraMessage>(new AstraMessage(mType, std::move(title), myFormat(fmt, std::forward<Args>(args)...)));
        }

        static inline std::string convertTypeToString(AstraMessageType type) {
            switch (type) {
                case AstraMessageType::Error:
                    return "Error";
                case AstraMessageType::Warning:
                    return "Warning";
                case AstraMessageType::Success:
                    return "Success";
                case AstraMessageType::Debug:
                    return "Debug";
                default:
                    return "Info";
            }
        }

        static inline AstraMessageType convertTypeFromString(const std::string_view& type) {
            if (type == "Error") return AstraMessageType::Error;
            if (type == "Warning") return AstraMessageType::Warning;
            if (type == "Success") return AstraMessageType::Success;
            if (type == "Debug") return AstraMessageType::Debug;
            return AstraMessageType::Info;
        }

        inline const std::string& getTitle() const { return m_title; }

        template<typename... Args>
        inline void setTitle(const std::string_view& title, Args&& ... args) {
            m_title = myFormat(title, std::forward<Args>(args)...);
        }

        inline const std::string& getContent() const { return m_content; }

        template<typename... Args>
        inline void setContent(const std::string_view& content, Args&& ... args) {
            m_content = myFormat(content, std::forward<Args>(args)...);
        }

        inline const auto& getTiming() const { return m_timing; }

        inline void setTiming(const std::chrono::system_clock::time_point& timing) { m_timing = timing; }

    private:
        explicit AstraMessage(AstraMessageType mType) : type(mType) {}

        AstraMessage(AstraMessageType mType, std::string&& msg) : type(mType), m_content(std::move(msg)) {}

        AstraMessage(AstraMessageType mType, std::string&& title, std::string&& msg) : type(mType), m_title(std::move(title)), m_content(std::move(msg)) {}
    };

} // Astra
