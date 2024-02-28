//
// Created by pierr on 15/08/2023.
//
#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <filesystem>

#include "Commons/utils/UUID.h"
#include "CpuEngine/engine/Device.h"

namespace Astra::CPU::Core {

    struct DeviceCreateData {
        UUID uuid;
        DeviceType type;
        std::string name;
        std::vector<UUID> connectedCpu;

        struct {
            size_t width = 256;
            size_t height = 240;
        } screenData;

        struct {
            UUID diskId;
            std::filesystem::path diskPath;
            bool readOnly;
        } diskData;

        struct {
            float masterVolume = 1;
        } audioData;
    };
}
