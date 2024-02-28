//
// Created by pierr on 26/08/2023.
//

#include "MemoryLoader.h"

#include <fstream>

#include "CpuEngine/manager/buses/DataBusManager.h"
#include "ViewerApp/CoreLib/System/Events.h"
#include "ViewerApp/Custom/ViewerConstants.h"

namespace Astra::UI::App {
    bool MemoryLoader::loadAllMemories(const std::filesystem::path& projectDirectory) {
        LOG_DEBUG("[MemoryLoader] loadAllMemories");

        bool isNoError = true;
        const auto memDir = projectDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::MEM_DIR;

        for (const auto& [busUUID, dataBus] : CPU::Core::DataBusManager::Get().GetDataBuses()) {
            isNoError &= loadMemory(dataBus, memDir / myFormat("{}.mem", busUUID));
        }

        LOG_DEBUG("[MemoryLoader] loadAllMemories END");
        return isNoError;
    }

    bool MemoryLoader::saveAllMemories(const std::filesystem::path& projectDirectory) {
        LOG_DEBUG("[MemoryLoader] saveAllMemories");
        bool isNoError = true;

        const auto memDir = projectDirectory / ViewerConstants::ASTRA_DIR / ViewerConstants::MEM_DIR;

        for (const auto& [busUUID, dataBus] : CPU::Core::DataBusManager::Get().GetDataBuses()) {
            isNoError &= saveMemory(dataBus, memDir / myFormat("{}.mem", busUUID));
        }

        LOG_DEBUG("[MemoryLoader] saveAllMemories END");
        return isNoError;
    }

    bool MemoryLoader::loadMemory(const Ref<CPU::Core::DataBus>& dataBus, const std::filesystem::path& filepath, size_t startAddr, size_t offset) noexcept {
        LOG_DEBUG("[MemoryLoader] loadMemory {}", dataBus->deviceUUID);

        if (!std::filesystem::exists(filepath)) {
            LOG_DEBUG("[MemoryLoader] loadMemory {} not exists, skipped", dataBus->deviceUUID);
            return false;
        }

        try {
            std::ifstream file(filepath, std::ios_base::binary);
            AstraException::assertV(file.good(), "Can not open file {}", filepath);
            if (offset != 0) {
                file.seekg(offset);
            }
            dataBus->GetRamMemory().loadFromStream(file, startAddr);
            file.close();
        } catch (const std::exception& e) {
            LOG_ERROR("[MemoryLoader] loadMemory error: {}", e.what());
            UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(AstraMessage::New(AstraMessageType::Error, "Load bus memory", e.what()));
            return false;
        }

        LOG_DEBUG("[MemoryLoader] loadMemory END");
        return true;
    }

    bool MemoryLoader::saveMemory(const Ref<CPU::Core::DataBus>& dataBus, const std::filesystem::path& filepath, size_t startAddr) noexcept {
        LOG_DEBUG("[MemoryLoader] saveMemory {}", dataBus->deviceUUID);

        try {
            std::ofstream file(filepath, std::ios_base::binary);
            AstraException::assertV(file.good(), "Can not open file {}", filepath);
            dataBus->GetRamMemory().dumpToStream(file);
            file.close();
        } catch (const std::exception& e) {
            LOG_ERROR("[MemoryLoader] saveMemory error: {}", e.what());
            UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(AstraMessage::New(AstraMessageType::Error, "Save bus memory", e.what()));
            return false;
        }

        LOG_DEBUG("[MemoryLoader] saveMemory END");
        return true;
    }
} // Astra