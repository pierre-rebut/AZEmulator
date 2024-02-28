//
// Created by pierr on 19/01/2024.
//

#include <fstream>

#include "DiskDevice.h"
#include "Commons/Log.h"

namespace Astra::CPU::Core {
    DiskDevice::DiskDevice(const DeviceCreateData& deviceInfo) : HardwareDevice(deviceInfo.uuid, DeviceType::DISK, deviceInfo.name),
                                                                 m_readOnly(deviceInfo.diskData.readOnly) {
        m_addrList = {{2,          0},
                      {m_diskSize, 2}};

        if (deviceInfo.diskData.diskId) {
            OpenDisk(deviceInfo.diskData.diskId, deviceInfo.diskData.diskPath);
        }
    }

    DiskDevice::~DiskDevice() {
        CloseDisk();
    }

    bool DiskDevice::OpenDisk(UUID diskId, const std::filesystem::path& diskPath) {
        CloseDisk();
        if (diskId == 0 || diskPath.empty()) {
            return false;
        }

        LOG_CPU_DEBUG("[DiskDevice] OpenDisk {} on disk {}", diskPath, deviceUUID);

        auto file = std::ifstream(diskPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            LOG_CPU_DEBUG("[DiskDevice] Failed to open disk {}", diskPath);
            return false;
        }

        m_diskSize = file.tellg();
        file.seekg(0, std::ios::beg);

        m_diskData = new BYTE[m_diskSize];
        file.read((char*) m_diskData, m_diskSize);
        file.close();

        m_diskId = diskId;
        m_diskPath = diskPath;

        m_status = DeviceStatus::READY;
        m_addrList = {{2,          0},
                      {m_diskSize, 2}};
        m_modified = false;

        LOG_CPU_DEBUG("[DiskDevice] OpenDisk END");
        return true;
    }

    void DiskDevice::CloseDisk() {
        if (!m_diskId) {
            return;
        }

        LOG_CPU_DEBUG("[RomDevice] CloseDisk {}", deviceUUID);
        SaveDisk();

        delete[] m_diskData;
        m_diskData = nullptr;
        m_diskSize = 0;
        m_diskId = 0;
        m_diskPath.clear();

        m_addrList = {{2,          0},
                      {m_diskSize, 2}};
        m_status = DeviceStatus::DISCONNECTED;

        LOG_CPU_DEBUG("[RomDevice] CloseDisk END");
    }

    void DiskDevice::SaveDisk() {
        if (m_modified) {
            auto file = std::ofstream(m_diskPath, std::ios::binary);
            if (file.is_open()) {
                file.write((char*) m_diskData, m_diskSize);
                file.close();
                LOG_CPU_TRACE("[IDiskDevice] Disk saved to {}", m_diskPath);
                m_modified = false;
            }
        }
    }

    void DiskDevice::Push(DataFormat fmt, size_t address, LARGE val) {
        if (m_readOnly || address < 2 || address > m_diskSize + 2) {
            return;
        }

        address -= 2;
        switch (fmt) {
            case DataFormat::Byte:
                m_diskData[address] = val;
                break;
            case DataFormat::Word:
                *(WORD*) (m_diskData + address) = val;
                break;
            case DataFormat::DWord:
                *(DWORD*) (m_diskData + address) = val;
                break;
            case DataFormat::Large:
                *(LARGE*) (m_diskData + address) = val;
                break;
        }

        m_modified = true;
    }

    LARGE DiskDevice::Fetch(DataFormat fmt, size_t address) {
        if (address == 0) {
            return m_status == DeviceStatus::READY;
        }

        if (address == 1) {
            return m_diskSize;
        }

        address -= 2;
        if (address >= m_diskSize) {
            return 0;
        }

        switch (fmt) {
            case DataFormat::Byte:
                return m_diskData[address];
            case DataFormat::Word:
                return *(WORD*) (m_diskData + address);
            case DataFormat::DWord:
                return *(DWORD*) (m_diskData + address);
            case DataFormat::Large:
                return *(LARGE*) (m_diskData + address);
        }
        return 0;
    }

    int DiskDevice::FetchReadOnly(size_t address) const {
        if (address == 0) {
            return m_status == DeviceStatus::READY;
        }

        if (address == 1) {
            return m_diskSize;
        }

        return m_diskData[address - 2];
    }
}
