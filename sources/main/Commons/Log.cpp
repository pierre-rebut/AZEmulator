//
// Created by pierr on 11/09/2021.
//

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Log.h"
#include "Commons/utils/YAMLimport.h"
#include "AstraException.h"

namespace Astra {
    constexpr char LOG_CONFIG_FILE[] = "log-config.yml";
    static const std::map<std::string, LogLevel, std::less<>> LOG_LEVEL_MAPPER = {
            {"trace",    LogLevel::trace},
            {"debug",    LogLevel::debug},
            {"info",     LogLevel::info},
            {"warn",     LogLevel::warn},
            {"err",      LogLevel::err},
            {"critical", LogLevel::critical},
            {"off",      LogLevel::off}
    };

    Ref<spdlog::logger> Log::sLogger;
    Ref<spdlog::logger> Log::sCpuLogger;

    void Log::Init() {
        try {
            initFirstConfigFile();
            loadLogConfigFile();
        } catch (const std::exception& e) {
            std::cerr << "Log config error : " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    static spdlog::sink_ptr createLoggerType(const YAML::Node& logger) {
        auto type = logger["Type"].as<std::string>();

        if (type == "console") {
            return CreateRef<spdlog::sinks::stdout_color_sink_mt>();
        }
        if (type == "file") {
            return CreateRef<spdlog::sinks::basic_file_sink_mt>(logger["Filename"].as<std::string>(),
                                                                logger["Truncate"].as<bool>());
        }

        throw AstraException("Invalid logger type {}: {}", type, LOG_CONFIG_FILE);
    }

    static std::vector<spdlog::sink_ptr> getLoggers(const std::string& loggerName, const YAML::Node& data) {
        std::vector<spdlog::sink_ptr> logSinks;

        for (auto logger: data[loggerName]) {
            if (!logger["Type"] || !logger["Pattern"]) {
                std::cerr << "LogConfig: invalid logger sink. skipping" << std::endl;
                continue;
            }

            auto logSink = createLoggerType(logger);
            logSink->set_pattern(logger["Pattern"].as<std::string>());
            logSinks.emplace_back(std::move(logSink));
        }

        return logSinks;
    }

    void Log::loadLogConfigFile() {
        YAML::Node data = YAML::LoadFile(LOG_CONFIG_FILE);
        if (!data["Version"] || !data["LogLevel"]) {
            throw AstraException("invalid config file: {}", LOG_CONFIG_FILE);
        }

        if (data["AppLoggers"]) {
            std::vector<spdlog::sink_ptr> logSinks = getLoggers("AppLoggers", data);

            if (!logSinks.empty()) {
                sLogger = CreateRef<spdlog::logger>("APP", begin(logSinks), end(logSinks));
                spdlog::register_logger(sLogger);
            }
        }

        if (data["CpuLoggers"]) {
            std::vector<spdlog::sink_ptr> logSinks = getLoggers("CpuLoggers", data);

            if (!logSinks.empty()) {
                sCpuLogger = CreateRef<spdlog::logger>("CPU", begin(logSinks), end(logSinks));
            }
        }

        SetLog(LOG_LEVEL_MAPPER.at(data["LogLevel"].as<std::string>()));
    }

    void Log::initFirstConfigFile() {
        if (!std::filesystem::exists(LOG_CONFIG_FILE)) {
            std::cout << "LogConfig: creating first log config file" << std::endl;

            std::ofstream fileOut(LOG_CONFIG_FILE);
            AstraException::assertV(fileOut.is_open(), "can not open file {}", LOG_CONFIG_FILE);

            YAML::Emitter outConfigLog;
            outConfigLog << YAML::BeginMap;
            outConfigLog << YAML::Key << "Version" << YAML::Value << 1;
            outConfigLog << YAML::Key << "LogLevel" << YAML::Value << "debug";
            outConfigLog << YAML::Key << "AppLoggers" << YAML::Value << YAML::BeginSeq;

            // console
            outConfigLog << YAML::BeginMap;
            outConfigLog << YAML::Key << "Type" << YAML::Value << "console";
            outConfigLog << YAML::Key << "Pattern" << YAML::Value << "%^[%T] [%l] %n: %v%$";
            outConfigLog << YAML::EndMap;

            // file
            outConfigLog << YAML::BeginMap;
            outConfigLog << YAML::Key << "Type" << YAML::Value << "file";
            outConfigLog << YAML::Key << "Filename" << YAML::Value << "APP.log";
            outConfigLog << YAML::Key << "Pattern" << YAML::Value << "[%T] [%l] %n: %v";
            outConfigLog << YAML::Key << "Truncate" << YAML::Value << true;
            outConfigLog << YAML::EndMap;

            outConfigLog << YAML::EndSeq;

            outConfigLog << YAML::Key << "CpuLoggers" << YAML::Value << YAML::BeginSeq;

            // console
            outConfigLog << YAML::BeginMap;
            outConfigLog << YAML::Key << "Type" << YAML::Value << "console";
            outConfigLog << YAML::Key << "Pattern" << YAML::Value << "%^[%T] [%l] %n: %v%$";
            outConfigLog << YAML::EndMap;

            // file
            outConfigLog << YAML::BeginMap;
            outConfigLog << YAML::Key << "Type" << YAML::Value << "file";
            outConfigLog << YAML::Key << "Filename" << YAML::Value << "CPU.log";
            outConfigLog << YAML::Key << "Pattern" << YAML::Value << "[%T] [%l] %n: %v";
            outConfigLog << YAML::Key << "Truncate" << YAML::Value << true;
            outConfigLog << YAML::EndMap;

            outConfigLog << YAML::EndSeq;
            outConfigLog << YAML::EndMap;

            fileOut << outConfigLog.c_str();
            fileOut.close();
        }
    }

    void Log::SetLog(LogLevel pLevel) {
        sLogger->set_level(static_cast<spdlog::level::level_enum>(pLevel));
        sLogger->flush_on(static_cast<spdlog::level::level_enum>(pLevel));
        sCpuLogger->set_level(static_cast<spdlog::level::level_enum>(pLevel));
        sCpuLogger->flush_on(static_cast<spdlog::level::level_enum>(pLevel));
    }
}
