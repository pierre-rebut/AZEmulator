//
// Created by pierr on 08/08/2023.
//
#pragma once

#include <queue>
#include <list>
#include <mutex>

#include "EngineLib/data/Base.h"
#include "Commons/utils/Singleton.h"
#include "Commons/AstraMessage.h"

#define NOTIFY_INFO(...) LogManager::Get().Notify(AstraMessage::New2(AstraMessageType::Info, __VA_ARGS__))
#define NOTIFY_DEBUG(...) LogManager::Get().Notify(AstraMessage::New2(AstraMessageType::Debug, __VA_ARGS__))
#define NOTIFY_SUCCESS(...) LogManager::Get().Notify(AstraMessage::New2(AstraMessageType::Success, __VA_ARGS__))
#define NOTIFY_WARN(...) LogManager::Get().Notify(AstraMessage::New2(AstraMessageType::Warning, __VA_ARGS__))
#define NOTIFY_ERROR(...) LogManager::Get().Notify(AstraMessage::New2(AstraMessageType::Error, __VA_ARGS__))

namespace Astra::CPU::Core {

    class LogManager : public Singleton<LogManager>
    {
    public:
        static constexpr const char* NAME = "LogManager";

    private:
        std::mutex m_mutex;
        std::queue<Ref<AstraMessage>> m_queue;

    public:
        void RetrieveMessages(std::list<Ref<AstraMessage>>& pMsg);

        void Notify(const Ref<AstraMessage>& newMessage);
    };

}
